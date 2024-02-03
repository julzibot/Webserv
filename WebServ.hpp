#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <csignal>
#include <fcntl.h>
#include <sys/time.h>
#include <map>
#include <list>
#include "conf_parsing/Config.hpp"
#include "RequestParsing.hpp"
#include "ResponseFormatting.hpp"

#define SOCKET 1
#define BIND 2
#define LISTEN 3
#define ACCEPT 4
#define RECV 5
#define SELECT 6
#define GETTIMEOFDAY 7

#define EXIT true
#define NO_EXIT false

#define CLIENT_TIMEOUT 30
#define CLIENT_TIMEOUT_CGI 10

#define RESETCLR "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

class WebServ {

private:

	Config						_config;
	HttpRequest					_request;
	int							_status;
	unsigned int				_arrsize;
	std::vector<sockaddr_in>	_saddr;
	std::vector<int>			_servsock; // all server sockets
	std::map<int, int>			_sockPortMap; // Socket and its associated port number

	// Socket loop 
	char   				_buff[2];
    std::string			_output;
    std::string			_filepath;
    std::string			_line;
    std::string			_prevReqPath;
    std::ifstream		_fs;
    ResponseFormatting	_formatter;
	char				**envp;

	// CLIENT
    struct sockaddr_in	_caddr;
    socklen_t			_caddrsize;
    int					_clientsock;
    int					_recvsize;

	fd_set				_currentSockets;
    fd_set				_readSockets;
	fd_set				_writeSockets;
    int					_maxFD;
    struct timeval		_timeoutSelect;

	std::vector<char>	_responseBody;

	// POST Method
	unsigned int		_maxBodySize;

	// Timeout management
	std::map<int, struct timeval>	_socketTimeoutMap;
	struct timeval					_currentTime;
	int								_socketTimeoutValue;

	// Init
	void	initSockets( const std::vector<int>& portNums );
	void	bindAndListen( const std::vector<int>& servsock,
				const std::vector<int>& portnums,
				const std::vector<sockaddr_in>& saddr,
				const unsigned int& arrsize );
	void	initSelectFDs( const unsigned int& size );

	void	checkClientTimeout(const struct timeval& currentTime,
	const int& keepAliveTimeout, const int& clientSock );
	bool	isServSock(const std::vector<int>& servsock, const int& sock);
	void	acceptNewConnection( const int& servSock );
	void	receiveFromExistingClient( const int& sockClient );
	std::string	get_response(std::string &filepath, int &status,
		HttpRequest &request, Config &config, std::vector<char>& body);
	
	// POST method
	void	receiveRequest( const int& sockClient,
				int& chunkSize, std::string& totalBuff );
	void	receiveBody( const int& sockClient );
	void	receiveMultiForm( const int& sockClient, std::string root, std::string boundary );
	void	receiveBinary( const int& sockClient, const std::string& endBoundary );
	void	receiveFile(const int& sockClient, const std::string& fileType,
		const std::string& filename, const std::string& root);
	void	sendToClient(const int& sockClient);
	void    socketFlush(const int& sockClient);

	// DELETE method
	void	deleteResource( const std::string& resource );

public:
	WebServ( const std::string& confFilenamePath, char **envp);
	~WebServ( void ) {};

	void	startServer( void );
};

void    printErrno(int func, bool ex);
