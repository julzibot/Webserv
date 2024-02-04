#include "WebServ.hpp"
#include "cgi/cgi.hpp"

// Function to run when CTRL-C is pressed
volatile sig_atomic_t    isTrue = 1;
void    sigHandler( int param ) { (void)param; isTrue = 0;}

void    printErrno(int func, bool ex)
{
    switch (func) {
        case SOCKET: std::cerr << "socket(): ";
        break;
        case BIND: std::cerr << "bind(): ";
        break;
        case LISTEN: std::cerr << "listen(): ";
        break;
        case RECV: std::cerr << "recv (): ";
        break;
        case SELECT: std::cerr << "select(): ";
        break;
        case GETTIMEOFDAY: std::cerr << "gettimeofday(): ";
        break;
        default: std::cerr << "Unknown func(): ";
    }

    switch (errno) {
        case EBADF: std::cerr << "errno ["<< errno << "]: Bad file descriptor" << std::endl;
        break;
        case ECONNABORTED: std::cerr << "errno [" << errno << "]: Connection was aborted" << std::endl;
        break;
        case EINVAL: std::cerr << "errno [" << errno << "]: Invalid argument" << std::endl;
        break;
        case ENOTSOCK: std::cerr << "errno [" << errno << "]: Descriptor is not a socket" << std::endl;
        break;
        case EWOULDBLOCK: std::cerr << "errno [" << errno << "]: Non-blocking socket" << std::endl;
        break;
        case EADDRINUSE: std::cerr << "errno [" << errno << "]: Address already in use" << std::endl;
        break;
        case EINTR: std::cerr << "errno [" << errno << "]: Interrupted system call" << std::endl;
        break;
        case ECONNRESET: std::cerr << "errno [" << errno << "]: Connection reset by peer" << std::endl;
        break;
        default: std::cerr << "errno: Unknown error code, should update: " << errno << std::endl;
    }

    if (ex) {
        exit(errno);
	}
}

void	WebServ::initSelectFDs( const unsigned int& size ) {

	_maxFD = _servsock[0];

    FD_ZERO(&_currentSockets);
	FD_ZERO(&_readSockets);
	FD_ZERO(&_writeSockets);
	for (unsigned int i = 0; i < size; ++i) {
  		FD_SET(_servsock[i], &_currentSockets);
		if (_servsock[i] > _maxFD)
			_maxFD = _servsock[i];
	}
}

WebServ::WebServ( const std::string& confFilenamePath, char **envp ) : _status(200),
	_arrsize(0), _filepath(""), _prevReqPath(""), _caddrsize(sizeof(_caddr)),
	_socketTimeoutValue(CLIENT_TIMEOUT) {

	try {
		_config = parse_config_file(confFilenamePath);
	}
	catch (std::invalid_argument &e) {
		std::cerr << RED << "Error: " << e.what() << RESETCLR << std::endl;
		exit(EXIT_FAILURE);
	}
	this->envp = envp;
	initSockets(_config.get_portnums());
	bindAndListen(_servsock, _config.get_portnums(), _saddr, _arrsize);
	initSelectFDs(_servsock.size());
	startServer();
}

/*** METHODS ***/

void	WebServ::initSockets( const std::vector<int>& portNums ) {

	_arrsize = portNums.size();
	_saddr.resize(_arrsize);
    for (unsigned int i = 0; i < _arrsize; i++) {
        _saddr[i].sin_family = AF_INET,
        _saddr[i].sin_addr.s_addr = INADDR_ANY,
        _saddr[i].sin_port = htons(portNums[i]);
    }
	for (unsigned int i = 0; i < _arrsize; ++i) {
		_servsock.push_back(socket(AF_INET, SOCK_STREAM, 0));
		fcntl(_servsock[i], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
		if (_servsock[i] == -1)
			printErrno(SOCKET, EXIT);
	}
}

void	WebServ::bindAndListen( const std::vector<int>& servsock, const std::vector<int>& portnums,
	const std::vector<sockaddr_in>& saddr, const unsigned int& arrsize ) {

	int option = 1;
	for (unsigned int i = 0; i < arrsize; ++i) {
		setsockopt(servsock[i], SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
		if (bind(servsock[i], (struct sockaddr*)&saddr[i], sizeof(saddr[i])) < 0)
			printErrno(BIND, EXIT);
		if (listen(servsock[i], SOMAXCONN) < 0)
			printErrno(LISTEN, EXIT);
		_sockPortMap[servsock[i]] = ntohs(saddr[i].sin_port);
		std::cout << "[SERVER] Now listening on port " << portnums[i] << std::endl;
	}
	std::map<int, int>::const_iterator	it;
	for (it = _sockPortMap.cbegin(); it != _sockPortMap.cend(); ++it)
		std::cout << "[" << it->first << "] = " << it->second << std::endl;
}

void	WebServ::checkClientTimeout(const struct timeval& currentTime,
	const int& keepAliveTimeout, const int& clientSock )
{
	if (currentTime.tv_sec - _socketTimeoutMap[clientSock].tv_sec > keepAliveTimeout) {
		std::cerr << RED << "Socket ["<< clientSock <<"]" << ": Timeout (set to "
			<< _socketTimeoutValue << "s)" << RESETCLR << std::endl;
		_request.http_version = "HTTP/1.1";
		_request.port_number = _sockPortMap[clientSock];
		std::string			emptyStr = "";
		std::vector<char>	emptyVec;
		_status = 408;
		_output = _formatter.format_response(_request, _status, emptyStr, _config, emptyVec);
		std::cout << CYAN << "Sending response:" << RESETCLR << std::endl;
		send(clientSock, _output.c_str(), _output.length(), 0);
		close(clientSock);
		if (clientSock == _maxFD)
			_maxFD -= 1;
		FD_CLR(clientSock, &_currentSockets);
		_socketTimeoutMap.erase(clientSock);
	}
}

bool	WebServ::isServSock(const std::vector<int>& servsock, const int& sock) {

	for (unsigned int i = 0; i < servsock.size(); ++i)
		if (servsock[i] == sock)
			return (true);
	return (false);
}

void	WebServ::acceptNewConnection( const int& servSock ) {	
	_clientsock = accept(servSock, (struct sockaddr*)&_caddr, (socklen_t*)&_caddrsize);
	if (_clientsock < 0 && errno != EWOULDBLOCK) {
		printErrno(ACCEPT, NO_EXIT);
		close(servSock);
	}
	else if (_clientsock > 2) {
		struct timeval	startSockTimeout;
		if (gettimeofday(&startSockTimeout, NULL) < 0)
			printErrno(GETTIMEOFDAY, EXIT);
		_socketTimeoutMap[_clientsock] = startSockTimeout;
		
		if (!isServSock(_servsock, _clientsock))
			std::cout << BOLD << "[SERVER] [socket: " << _clientsock
				<< "] New client connected with success" << RESETCLR << std::endl;
		fcntl(_clientsock, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
		if (_clientsock > _maxFD)
			_maxFD = _clientsock;
		FD_SET(_clientsock, &_currentSockets);
		_sockPortMap[_clientsock] = _sockPortMap[servSock];
	}
}


std::string	WebServ::get_response(std::string &filepath, int &status,
	HttpRequest &request, Config &config, std::vector<char>& body)
{
	if (!filepath.empty())
	{
		std::string	extension = filepath.substr(filepath.find_last_of(".") + 1);
		std::string cgiExecPath = config.get_cgi_type(extension);
		std::string reqHost = request.hostIP;
		std::string p = request.path.substr(0, request.path.find('/', 1));

		if (status == 200 && (extension == "py" || extension == "php"))
		{
			CGI *cgi = new CGI(this->envp, cgiExecPath);
			cgi->execute_cgi(request, cgi, filepath, status, body);
			std::string	bodyStr(body.begin(), body.end());
			std::deque<std::string> status_infos = get_status_infos(status,
				bodyStr, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);
			if (status != 200) {
				ResponseFormatting::parse_body(status_infos[0], status, body);
			}
			std::string headers = ResponseFormatting::parse_cgi_headers(request.http_version, body.size(),
					status, status_infos);
			std::string response = headers + "\r\n";
			delete cgi;
			return (response);
		}
	}
	return (ResponseFormatting::format_response(request, status, filepath, config, body));
}

void	WebServ::sendToClient(const int& sockClient)
{
	_output = WebServ::get_response(_filepath, _status, _request, _config, _responseBody);
	std::cout << CYAN << "[socket: " << sockClient << "] Sending response:" << RESETCLR << std::endl;
	std::cout << _output.c_str() << std::endl;

	std::vector<char>	fullResponse(_output.begin(), _output.end());

	size_t	totalSent = 0;
	int		bytesSent = 0;

	if (!_responseBody.empty()) {
		fullResponse.insert(fullResponse.end(), _responseBody.begin(), _responseBody.end());
	}
	size_t	bytesLeft = fullResponse.size();

	while (totalSent < fullResponse.size()) {
		bytesSent = send(sockClient, fullResponse.data() + totalSent, bytesLeft, 0);
		if (bytesSent != -1) {
			totalSent += bytesSent;
			bytesLeft -= bytesSent;
		}
	}
	FD_CLR(sockClient, &_writeSockets);
	FD_SET(sockClient, &_readSockets);
	fullResponse.clear();
	if (!_request.keepalive) {
		close(sockClient);
		if (sockClient == _maxFD)
			_maxFD -= 1;
		FD_CLR(sockClient, &_currentSockets);
	}
	_request.body.clear();
	_responseBody.clear();
	_request.fullRequest.clear();
	_output.clear();
}

void	WebServ::removeUntilCRLF( std::vector<char>& request )
{
	const char	*crlf = "\r\n\r\n";

	std::vector<char>::iterator	it = std::search(request.begin(), request.end(), crlf, crlf + 4);
	if (it != request.end())
		request.erase(request.begin(), it + 4);
}

bool	WebServ::receiveRequest(const int& sockClient, std::vector<char> &totalBuff )
{
	_recvsize = 0;
	char		buff[4096];
	int			bytesRead = 1;
	const char	*crlf = "\r\n\r\n";

	totalBuff.clear();

    HttpRequest tempRequest;
	bool		headersParsed = false;
    while (true) {
        bytesRead = 1;
        while (bytesRead > 0)
        {
            std::memset(buff, '\0', 4096);
            bytesRead = recv(sockClient, buff, 4096, 0);
            if (bytesRead > 0) {
                _recvsize += bytesRead;
				for (int j = 0; j < bytesRead; j++)
					totalBuff.push_back(buff[j]);
			}
        }
		if (!headersParsed && std::search(totalBuff.begin(),
				totalBuff.end(), crlf, crlf + 4) != totalBuff.end())
		{
			HttpRequestParse::parse(tempRequest, totalBuff, _sockPortMap[sockClient]);
			if (!tempRequest.accepted_method)
			{
				_status = 405;
			}
			headersParsed = true;
		}
        if (headersParsed && tempRequest.content_length > 0
			&& static_cast<size_t>(tempRequest.content_length) <= totalBuff.size())
		{	
        	break;
		}
		else if (tempRequest.content_length <= 0)
			break;
    }
	_request = tempRequest;
	_request.body = totalBuff;
	if (!headersParsed)
		return (false);
	return (true);
}

void	WebServ::receiveFromExistingClient(const int& sockClient)
{
	struct timeval	timeoutUpdate;
	if (gettimeofday(&timeoutUpdate, NULL) < 0)
		printErrno(GETTIMEOFDAY, EXIT);
	_socketTimeoutMap[sockClient] = timeoutUpdate;

	std::vector<char>	totalBuff;

	bool	recvBool = receiveRequest(sockClient, totalBuff);

	if (!recvBool) {
		std::cout << YELLOW << "[socket: " << sockClient << "] Client has disconnected" << RESETCLR << std::endl;
		close(sockClient);
		if (sockClient == _maxFD)
			_maxFD -= 1;
		FD_CLR(sockClient, &_currentSockets);
	}
	else {
		std::cout << MAGENTA << "[Server] [socket: " << sockClient << "] Receiving request from client:" << RESETCLR << std::endl;
		std::cout << _request.strHeaders << std::endl;
		_request.strHeaders.clear();

		std::string reqHost = _request.headers["Host"];
		reqHost = reqHost.substr(0,reqHost.find(':'));
		reqHost.erase(std::remove(reqHost.begin(), reqHost.end(), '\r'), reqHost.end());
		request_ip_check(reqHost, _config, _status);
		_request.hostIP = reqHost;

		if (_request.method == "POST") {
			if (_request.content_length > _config.get_max_body())
				_status = 413;
			else
				receiveBody();
		}
		else if (_request.method == "DELETE")
			deleteResource(_request.path);
		if (_status == 200) {
			_filepath = get_file_path(_request, _config, _status);
		}
	}
	FD_CLR(sockClient, &_readSockets);
	FD_SET(sockClient, &_writeSockets);
	_request.fullRequest.clear();
	totalBuff.clear();
}

void	WebServ::startServer( void )
{
	signal(SIGINT, sigHandler);
    while (isTrue)
    {
        _status = 200;
		_readSockets = _currentSockets;
		if (select(_maxFD + 1, &_readSockets, &_writeSockets, NULL, NULL) < 0) {
			std::cout << "errno: " << errno << std::endl;
			printErrno(SELECT, NO_EXIT);
			break;
		}

		for (int i = 0; i < _maxFD + 1; ++i)
		{
			if (isServSock(_servsock, i) && FD_ISSET(i, &_readSockets)) // Accepting new connection
				acceptNewConnection(i);
			else if (FD_ISSET(i, &_readSockets))
				receiveFromExistingClient(i);
			if (FD_ISSET(i, &_writeSockets))
				sendToClient(i);
			else if (FD_ISSET(i, &_currentSockets) && !isServSock(_servsock, i)) { // Check keep-alive timeout
				if (gettimeofday(&_currentTime, NULL) < 0)
					printErrno(GETTIMEOFDAY, EXIT);
				checkClientTimeout(_currentTime, _socketTimeoutValue, i);
			}
    	}
	}
	for (int i = 0; i < _maxFD + 1; ++i) {
		if (FD_ISSET(i, &_currentSockets)) {
			FD_CLR(i, &_currentSockets);
			close(i);
		}
	}
	for (unsigned int i = 0; i < _servsock.size(); ++i) {
		close(_servsock[i]);
	}
}
