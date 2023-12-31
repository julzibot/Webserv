/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/17 19:35:25 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/18 19:29:27 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/socket.h>
#include <sys/types.h>
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

#define RESETCLR "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"

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
	char   				_buff[4096];
    std::string			_output;
    std::string			_filepath;
    std::string			_line;
    std::string			_prevReqPath;
    std::ifstream		_fs;
    ResponseFormatting	_formatter;

	// CLIENT
    struct sockaddr_in	_caddr;
    socklen_t			_caddrsize;
    int					_clientsock;
    int					_recvsize;

	fd_set            _currentSockets;
    fd_set            _readySockets;
    int               _maxFD;
    struct timeval    _timeoutSelect;

	// Timeout management
	std::map<int, struct timeval>	_socketTimeoutMap;
	struct timeval					_currentTime;
	int								_socketTimeoutValue;

	// Init
	void	initSockets( const std::vector<int>& portNums );
	void	bindAndListen( const std::vector<int>& servsock, const std::vector<int>& portnums,
		const std::vector<sockaddr_in>& saddr, const unsigned int& arrsize );
	void	initSelectFDs( const unsigned int& size );

	void	checkClientTimeout(const struct timeval& currentTime,
		const int& keepAliveTimeout, const int& clientSock );
	bool	isServSock(const std::vector<int>& servsock, const int& sock);
	void	acceptNewConnection( const int& servSock );
	void	receiveFromExistingClient( const int& sockClient );
	
public:

	WebServ( const std::string& confFilenamePath );
	~WebServ( void ) {};

	void	startServer( void );

};
