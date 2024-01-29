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

    if (ex)
        exit(errno);
}

void	WebServ::initSelectFDs( const unsigned int& size ) {

	_timeoutSelect.tv_usec = 20;
	_timeoutSelect.tv_sec = 0;
	_maxFD = _servsock[0];

    FD_ZERO(&_currentSockets);
	for (unsigned int i = 0; i < size; ++i) {
  		FD_SET(_servsock[i], &_currentSockets);
		if (_servsock[i] > _maxFD)
			_maxFD = _servsock[i];
	}
}

WebServ::WebServ( const std::string& confFilenamePath, char **envp ) : _status(200),
	_arrsize(0), _filepath(""), _prevReqPath(""), _caddrsize(sizeof(_caddr)),
	_maxBodySize(UINT_MAX), _socketTimeoutValue(CLIENT_TIMEOUT) {

	try {
		_config = parse_config_file(confFilenamePath);
	}
	catch (std::invalid_argument &e) {
		std::cerr << "Error: " << e.what() << std::endl;
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
		std::cerr << RED << "Socket ["<< clientSock <<"]" << ": Timeout (set to " << _socketTimeoutValue << "s)" << RESETCLR << std::endl;
		_request.http_version = "HTTP/1.1";
		_request.port_number = _sockPortMap[clientSock];
		std::string	emptyStr = "";
		_status = 408;
		_output = _formatter.format_response(_request, _status, emptyStr, _config);
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
			std::cout << BOLD << "[SERVER] [socket: " << _clientsock << "] New client connected with success" << RESETCLR << std::endl;
		fcntl(_clientsock, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
		if (_clientsock > _maxFD)
			_maxFD = _clientsock;
		FD_SET(_clientsock, &_currentSockets);
		_sockPortMap[_clientsock] = _sockPortMap[servSock];
	}
}

void	WebServ::receiveRequest(const int& sockClient, int& chunkSize, std::string& headers ) {
	// POST:
	// if Content-Length > limit_max_body_size send 413 Content Too Large
	// create a file with type specified in Content-Type
	// if \r\n\r\n (CRLF) encountered, headers end, body begins
	while (chunkSize > 0) {
		if (headers.find("\r\n\r\n") != NPOS)
			break;
		memset(_buff, 0, 2);
		chunkSize = recv(sockClient, _buff, 1, 0);
		if (chunkSize == 0)
			break;
		_recvsize += chunkSize;
		headers.append(_buff);
	}
}

std::string	WebServ::get_response(std::string &filepath, int &status,
	HttpRequest &request, Config &config)
{
	if (!filepath.empty())
	{
		std::string	extension = filepath.substr(filepath.find_last_of(".") + 1);
		std::string cgiExecPath = config.get_cgi_type(extension);
		std::string reqHost = request.hostIP;
		std::string p = request.path.substr(0, request.path.find('/', 1));

		int	status;
		if (extension == "py" || extension == "php")
		{
			CGI *cgi = new CGI(this->envp, cgiExecPath);
			std::string body = cgi->execute_cgi(request, cgi, filepath, status);
			std::deque<std::string> status_infos = ResponseFormatting::get_status_infos(status,
				body, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);
			if (status != 200) {
				body = ResponseFormatting::parse_body(status_infos[0], status);
			}
			std::string headers = ResponseFormatting::parse_cgi_headers(request.http_version, body.length(),
					status, status_infos);
			std::string response = headers + "\r\n" + body;
			delete cgi;
			return (response);
		}
	}
	return (ResponseFormatting::format_response(request, status, filepath, config));
}

void	WebServ::receiveFromExistingClient(const int& sockClient)
{
	struct timeval	timeoutUpdate;
	if (gettimeofday(&timeoutUpdate, NULL) < 0)
		printErrno(GETTIMEOFDAY, EXIT);
	_socketTimeoutMap[sockClient] = timeoutUpdate;

	std::string	totalBuff;
	int			chunkSize = 1;
	
	memset(_buff, 0, 2);
	_recvsize = recv(sockClient, _buff, 1, 0);
	totalBuff.append(_buff);

	if (_recvsize == 0) {
		close(sockClient);
		if (sockClient == _maxFD)
			_maxFD -= 1;
		FD_CLR(sockClient, &_currentSockets);
	}
	else if (_recvsize > 0) {
		receiveRequest(sockClient, chunkSize, totalBuff);
		_request = HttpRequestParse::parse(totalBuff, _sockPortMap[sockClient]);

		std::string reqHost = _request.headers["Host"];
		reqHost = reqHost.substr(0,reqHost.find(':'));
		reqHost.erase(std::remove(reqHost.begin(), reqHost.end(), '\r'), reqHost.end());
		request_ip_check(reqHost, _config, _status);
		_request.hostIP = reqHost;

		totalBuff.clear();
		_filepath = get_file_path(_request, _config, _status);
		std::cout << "FILEPATH: " << _filepath << " STATUS: " << _status << std::endl;
		if (_request.method == "POST") {
			if (_request.content_length > _maxBodySize)
				_status = 413;
			else
				receiveBody(sockClient);
		}
		else if (_request.method == "DELETE")
			deleteResource(_request.path);
		_output = WebServ::get_response(_filepath, _status, _request, _config);
		send(sockClient, _output.c_str(), _output.length(), 0);
		if (!_request.keepalive) {
			close(sockClient);
			if (sockClient == _maxFD)
				_maxFD -= 1;
			FD_CLR(sockClient, &_currentSockets);
		}
	}
}

void	WebServ::startServer( void ) {

	signal(SIGINT, sigHandler);
    while (isTrue)
    {
        _status = 200;
		_readySockets = _currentSockets;
		if (select(_maxFD + 1, &_readySockets, NULL, NULL, &_timeoutSelect) < 0)
			printErrno(SELECT, EXIT);

		for (int i = 0; i < _maxFD + 1; ++i)
		{
			if (isServSock(_servsock, i) && FD_ISSET(i, &_readySockets)) // Accepting new connection
				acceptNewConnection(i);
			else if (FD_ISSET(i, &_readySockets))
				receiveFromExistingClient(i);
			else if (FD_ISSET(i, &_currentSockets) && !isServSock(_servsock, i)) { // Check keep-alive timeout
				if (gettimeofday(&_currentTime, NULL) < 0)
					printErrno(GETTIMEOFDAY, EXIT);
				checkClientTimeout(_currentTime, _socketTimeoutValue, i);
			}
    	}
	}
	for (int i = 0; i < _maxFD + 1; ++i) {
		close(i);
		FD_CLR(i, &_currentSockets);
	}
	for (unsigned int i = 0; i < _servsock.size(); ++i) {
		close(_servsock[i]);
	}
}
