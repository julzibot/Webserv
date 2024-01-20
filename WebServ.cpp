/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/17 19:37:42 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/19 20:02:27 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

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
        case RECV: std::cerr << "recv(): ";
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

WebServ::WebServ( const std::string& confFilenamePath ) : _status(200),
	_arrsize(0), _filepath(""), _prevReqPath(""), _caddrsize(sizeof(_caddr)),
	_isPostMethod(false), _contentLength(-1), _maxBodySize(UINT_MAX), _socketTimeoutValue(90) {

	try {
		_config = parse_config_file(confFilenamePath);
	}
	catch (std::invalid_argument &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	initSockets(_config.get_portnums());
	bindAndListen(_servsock, _config.get_portnums(), _saddr, _arrsize);
	initSelectFDs(_servsock.size());
	startServer();
}

/* METHODS */

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

void	WebServ::checkClientTimeout(const struct timeval& currentTime,
	const int& keepAliveTimeout, const int& clientSock ) {

	// std::cout << "[socket: " << _clientsock << "] - " << currentTime.tv_sec - _socketTimeoutMap[clientSock].tv_sec << "s" << std::endl;
	if (currentTime.tv_sec - _socketTimeoutMap[clientSock].tv_sec > keepAliveTimeout) {
		std::cerr << RED << "Socket ["<< clientSock <<"]" << ": Timeout (set to " << _socketTimeoutValue << "s)" << RESETCLR << std::endl;
		_request.http_version = "HTTP/1.1";
		_request.port_number = _sockPortMap[clientSock];
		std::string	emptyStr = "";
		_status = 408;
		_output = _formatter.format_response(_request, _status, emptyStr, _config);
		std::cout << _output.c_str() << std::endl;
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

// void	process_method(std::string &filepath, int &status, HttpRequest &request, Config &config) {
	
// 	std::string extension;

// 	if (!filepath.empty()) {
// 		std::string	extension = filepath.substr(filepath.find_last_of(".") + 1);
// 		std::string cgiExecPath = config.get_cgi_type(extension);
// 		if (cgiExecPath != "")
// 			request.cgi = true;
// 	}
// 	if (request.method == "POST" && request.cgi == false)
// 		// MILAN AND JULES
// 	else if (request.method == "GET" && request.cgi == true)
// 		// TOSH
// 	else if (request.method == "POST" && request.cgi == true)
// 		// TOSH
// 	else if (request.method == "DELETE")
// 		// cgi true OR false
// }

void	WebServ::receiveRequest(const int& sockClient, int& chunkSize, std::string& headers ) {

	// POST:
	// if Content-Length > limit_max_body_size send 413 Content Too Large
	// create a file with type specified in Content-Type
	// if \r\n\r\n (CRLF) encountered, headers end, body begins
	while (chunkSize > 0) {
		if (headers.find("POST") != NPOS && headers.find("\r\n\r\n") != NPOS) {
			_isPostMethod = true;
			break;
		}
		memset(_buff, 0, 2);
		chunkSize = recv(sockClient, _buff, 1, 0);
		if (chunkSize == 0)
			break;
		_recvsize += chunkSize;
		headers.append(_buff);
	}
}

std::string	extractBetweenChars(std::string str, char c) {

	if (str.empty())
		return ("");
	std::string::size_type	start = str.find(c);
	if (start == std::string::npos)
		return ("");
	++start;
	std::string::size_type	end = start;

	while (end != str.length()) {
		if (str.at(end) == c)
			break;
		++end;
	}
	if (end == str.length() || start == end - 1)
		return ("");
	return (str.substr(start, end - start));
}

void	WebServ::receiveBinary(const int& sockClient, const std::string& endBoundary) {

	int		chunkSize = 1;
	char	binChar[30];
	
	while (chunkSize > 0) {
		memset(binChar, 0, 2);
		chunkSize = recv(sockClient, binChar, 1, 0);
		if (chunkSize == 0)
			break;
		_binaryBody.push_back(binChar[0]);
	}
	std::vector<char>::iterator	boundIt = std::search(_binaryBody.begin(),
		_binaryBody.end(), endBoundary.begin(), endBoundary.end());
	if (boundIt != _binaryBody.end())
		_binaryBody.erase(boundIt, _binaryBody.end());
}

void	WebServ::receiveMultiForm( const int& sockClient, std::string root, std::string boundary ) {

	int			chunkSize = 1;
	char		formDataBody[2];
	std::string	formHeaderData;

	while (chunkSize > 0) {
		memset(formDataBody, 0, 2);
		chunkSize = recv(sockClient, formDataBody, 1, 0);
		if (chunkSize == 0)
			break;

		formHeaderData.append(formDataBody);
		if (formHeaderData.find("\r\n\r\n") != NPOS) {
			receiveBinary(sockClient, "--" + boundary + "--");
			break;
		}
	}

	// parse form data:
	// - supported body type
	// - filename
	std::vector<std::string>	formHeaders;
	std::istringstream			formStream(formHeaderData);
	std::string					tmpLine;
	
	while (std::getline(formStream, tmpLine)) {
		tmpLine.erase(tmpLine.find("\r"));
		formHeaders.push_back(tmpLine);
	}

	// Parse body form info
	std::string	inputName;
	std::string	fileExt;
	std::string	fileType;

	for (size_t i = 0; i < formHeaders.size(); ++i) {

		if (formHeaders[i].find(" name=\"") != NPOS)
			inputName = extractBetweenChars(formHeaders[i].substr(formHeaders[i].find(" name=\"")), '\"');
		if (formHeaders[i].find(" filename=\"") != NPOS)
			fileExt = extractBetweenChars(formHeaders[i].substr(formHeaders[i].find(" filename=\"")), '\"');
		if (formHeaders[i].find("Content-Type: ") != NPOS) {
			fileType = formHeaders[i].substr(formHeaders[i].find("Content-Type: ") + 14);
			if (fileType != "image/jpeg" && fileType != "image/png")
			{
				std::cerr << "Error: Unsupported file type" << std::endl;
				_status = 415; // Unsupported Media Type
				return;
			}
		}
	}
	std::string	filename = "/" + inputName;
	if (fileType == "image/jpeg")
		filename += ".jpg";
	else if (fileType == "image/png")
		filename += ".png";
	root += filename;
	std::ofstream	newFile;
	newFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	try {
		// Check if file exists
		struct stat	fileInfos;
		if (stat(root.c_str(), &fileInfos) != -1) {
			_status = 409; // Conflict
			return;
		}
		
		newFile.open(root, std::ios::binary);
		for (std::vector<char>::iterator it = _binaryBody.begin(); it != _binaryBody.end(); ++it)
			newFile << *it;
		newFile.close();
		_binaryBody.clear();
	}
	catch (const std::ofstream::failure& e) {
		std::cerr << RED << "Caught exception: receiveMultiForm(): '" << e.what()
			<< "' while receiving file on port: " << _request.port_number << RESETCLR << std::endl;
		_status = 500;
		return;
	}

	// // Create timestamp
	// struct timeval	currentTime;
	// if (gettimeofday(&currentTime, NULL) < 0)
	// 	printErrno(GETTIMEOFDAY, EXIT);

	// time_t		now = currentTime.tv_sec;
    // struct tm	*tm_now = localtime(&now);
    // char		date_buffer[20];

    // strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d %H:%M:%S", tm_now);

}

void	WebServ::receiveFileOnly(const int& sockClient, const std::string& fileType,
	const std::string& root) {

	char		binChar[2];
	int			chunkSize = 1;
	std::string	fileName;

	if (fileType == "image/jpeg")
		fileName = root + "/unknown_name.jpg";
	else if (fileType == "image/png")
		fileName = root + "/unknown_name.png";

	while (chunkSize > 0) {
		memset(binChar, 0, 2);
		chunkSize = recv(sockClient, binChar, 1, 0);
		if (chunkSize == 0)
			break;
		_binaryBody.push_back(binChar[0]);
	}
	std::ofstream	newFile;
	newFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	try {
		newFile.open(fileName, std::ios::binary);

		// if file exists, status code = 409 Conflict
		for (std::vector<char>::iterator it = _binaryBody.begin(); it != _binaryBody.end(); ++it)
			newFile << *it;
		newFile.close();
	}
	catch (const std::ofstream::failure& e) {
		newFile.close();
		std::cerr << RED << "Caught exception: receiveFileOnly(): '" << e.what()
			<< "' while receiving file on port: " << _request.port_number << RESETCLR << std::endl;
		_status = 500; // Internal Server Error
	}
	_binaryBody.clear();
	return;
}

void	WebServ::receiveBody( const int& sockClient ) {

	std::string p = _request.path.substr(0, _request.path.find('/', 1));
	std::string	root = _config.getServMain(_request.port_number, p, true)["root"];
	if (root.empty()) {
		std::cerr << "receiveBody: 'root' not found" << std::endl;
		_status = 500; // 500 Internal Server Error
		return;
	}

	std::string			fileType;
	std::string			fileName;
	strstrMap::iterator contentTypeIt = _request.headers.find("Content-Type");
	if (contentTypeIt == _request.headers.end()) {

		std::cerr << "Error: Cannot process POST request: No 'Content-Type' in request headers" << std::endl;
		_status = 422; // Unprocessable Content
		return;
	}
	// if File or Form (Content-Type)
	fileType = contentTypeIt->second;
	if (fileType.find("multipart/form-data") != NPOS) {

		std::string	boundary;
		try {
			boundary.append(fileType.substr(fileType.find("boundary=") + 9, fileType.length() - 1));
		}
		catch (std::exception &e) {
			std::cerr << "Error: Cannot process POST request: No boundary inside multiform/data." << std::endl;
			_status = 400; // Bad request
			return;
		}
		receiveMultiForm(sockClient, root, boundary);
	}
	else
		receiveFileOnly(sockClient, fileType, root);
}

void	WebServ::receiveFromExistingClient(const int& sockClient ) {

	struct timeval	timeoutUpdate;
	if (gettimeofday(&timeoutUpdate, NULL) < 0)
		printErrno(GETTIMEOFDAY, EXIT);
	_socketTimeoutMap[sockClient] = timeoutUpdate;

	std::string	totalBuff;
	int			chunkSize = 1;
	
	std::cout << BOLD << "[SERVER] [socket: " << sockClient << "] Receiving from existing client" << RESETCLR << std::endl;
	memset(_buff, 0, 2);
	_recvsize = recv(sockClient, _buff, 1, 0);
	totalBuff.append(_buff);

	if (_recvsize < 0 && errno != EWOULDBLOCK) {
		printErrno(RECV, NO_EXIT);
		std::cerr << BOLD << "[SERVER] Error encountered while receiving message" << RESETCLR << std::endl;
	}
	else if (_recvsize == 0 && errno != EWOULDBLOCK) {
		std::cout << BOLD << "[SERVER] [socket: " << sockClient << "] Client disconnected" << RESETCLR << std::endl;
		close(sockClient);
		if (sockClient == _maxFD)
			_maxFD -= 1;
		FD_CLR(sockClient, &_currentSockets);
	}
	else if (_recvsize > 0) {
		receiveRequest(sockClient, chunkSize, totalBuff);
		std::cout << BOLD << "[SERVER] [socket: " << sockClient << "] Receiving request:" << RESETCLR << std::endl;
		std::cout << totalBuff << std::endl;
		_request = HttpRequestParse::parse(totalBuff, _sockPortMap[sockClient]);
		_filepath = get_file_path(_request, _config, _status);
		if (_isPostMethod) { // POST

			strstrMap::iterator	headerIt = _request.headers.find("Content-Length");
			if (headerIt != _request.headers.end()) {
				_contentLength = std::atoi(headerIt->second.c_str());
			}
			if (_contentLength > _maxBodySize)
				_status = 413;
			else
				receiveBody(sockClient);
			_isPostMethod = false;
		}
		_output = ResponseFormatting::format_response(_request, _status, _filepath, _config);
		std::cout << CYAN << "Sending response:" << RESETCLR << std::endl;
		std::cout << _output.c_str() << std::endl;
		send(sockClient, _output.c_str(), _output.length(), 0);
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
	std::cout << BOLD << RED << "Closing all sockets:" << RESETCLR;
	for (int i = 0; i < _maxFD + 1; ++i) {
		std::cout << " " << i;
		close(i);
		FD_CLR(i, &_currentSockets);
	}
	std::cout << std::endl;
	for (unsigned int i = 0; i < _servsock.size(); ++i) {
		close(_servsock[i]);
	}
}
