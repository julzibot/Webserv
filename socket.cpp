/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/16 22:21:40 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "RequestParsing.hpp"
#include "ResponseFormatting.hpp"
#include "conf_parsing/Config.hpp"
#include "conf_parsing/DirectiveParsing.h"
#include <csignal>
#include <fcntl.h>

#define SOCKET 1
#define BIND 2
#define LISTEN 3
#define ACCEPT 4
#define RECV 5
#define SELECT 6

#define EXIT true
#define NO_EXIT false

volatile sig_atomic_t    isTrue = 1;

// Function to run when CTRL-C is pressed
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

bool	isServSock(const std::vector<int>& servsock, const int& sock) {

	for (unsigned int i = 0; i < servsock.size(); ++i)
		if (servsock[i] == sock)
			return (true);
	return (false);
}

void	init_sockets(Config const &config, std::vector<sockaddr_in> &saddr, std::vector<int> &servsock, \
		std::map<int, int>	&SockPortMap)
{
	unsigned int	arrsize = config.get_portnums().size();
	int	option = 1;

	for (unsigned int i = 0; i < arrsize; i++)
    {
		// SADDR
		if (SockPortMap.find(servsock[i]) == SockPortMap.end())
		{
			saddr[i].sin_family = AF_INET,
			saddr[i].sin_addr.s_addr = INADDR_ANY,
			saddr[i].sin_port = htons(config.get_portnums()[i]);

			//  SOCKET VECTOR
			servsock.push_back(socket(AF_INET, SOCK_STREAM, 0));
			fcntl(servsock[i], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
			if (servsock[i] == -1)
				printErrno(SOCKET, EXIT);

			// SOCKET->PORT MAP
			setsockopt(servsock[i], SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
			if (bind(servsock[i], (struct sockaddr*)&saddr[i], sizeof(saddr[i])) < 0)
				printErrno(BIND, EXIT);
			if (listen(servsock[i], SOMAXCONN) < 0)
				printErrno(LISTEN, EXIT);
			SockPortMap[servsock[i]] = ntohs(saddr[i].sin_port);
			std::cout << "[SERVER] Now listening on port " << config.get_portnums()[i] << std::endl;
		}
    }
}

int main (int argc, char ** argv)
{
	if (argc > 2) {
	std::cout << "Error: too many arguments.";
	std::cout << " Either input config file or nothing as argument" << std::endl;
	return (1); }
	std::string conf_filename;
	if (argc == 2) conf_filename = "server_files/" + std::string(argv[1]);
	else conf_filename = "server_files/webserv.conf";

	Config	config;
	try { config = parse_config_file(conf_filename);}
	catch (std::invalid_argument &a)
	{std::cerr << "Error: " << a.what() << std::endl; return (1);}
    int				status = 200;
    HttpRequest		request;

    std::vector<sockaddr_in>	saddr(config.get_portnums().size());
	std::vector<int>	servsock;
	std::map<int, int>	SockPortMap;
	init_sockets(config, saddr, servsock, SockPortMap);

	std::map<int, int>::iterator	it;
	for (it = SockPortMap.begin(); it != SockPortMap.end(); ++it)
		std::cout << "[" << it->first << "] = " << it->second << std::endl;

    //WAITING TO ACCEPT
    char   				buff[4096];
    std::string			output;
    std::string			filepath;
    std::string			line;
    std::string			prevReqPath = "";
    std::ifstream		fs;
    ResponseFormatting 	formatter;

    // CLIENT
    struct sockaddr_in	caddr;
    socklen_t			caddrsize = sizeof(caddr);
    int					clientsock;
    int					recvsize;

    fd_set            currentSockets;
    fd_set            readySockets;
    int               maxFD = servsock[0];
    struct timeval    timeoutSocket;
    timeoutSocket.tv_usec = 20;
    timeoutSocket.tv_sec = 0;

    FD_ZERO(&currentSockets);
	for (unsigned int i = 0; i < servsock.size(); ++i) {
  		FD_SET(servsock[i], &currentSockets);
		if (servsock[i] > maxFD)
			maxFD = servsock[i];
	}

    signal(SIGINT, sigHandler);
    while (isTrue)
    {
        status = 200;
		readySockets = currentSockets;
		if (select(maxFD + 1, &readySockets, NULL, NULL, &timeoutSocket) < 0)
			printErrno(SELECT, EXIT);
		for (int i = 0; i < maxFD + 1; ++i)
		{
			if (isServSock(servsock, i) && FD_ISSET(i, &readySockets)) { // Accepting new connection
				clientsock = accept(i, (struct sockaddr*)&caddr, (socklen_t*)&caddrsize);
				if (clientsock < 0 && errno != EWOULDBLOCK) {
					printErrno(ACCEPT, NO_EXIT);
					close(i);
				}
				else if (clientsock > 2) {
					if (!isServSock(servsock, clientsock))
						std::cout << "\033[1m[SERVER] [socket: " << clientsock << "] New client connected with success\033[0m" << std::endl;
					fcntl(clientsock, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
					if (clientsock > maxFD)
						maxFD = clientsock;
					FD_SET(clientsock, &currentSockets);
					SockPortMap[clientsock] = SockPortMap[i];
				}
			}
			else if (FD_ISSET(i, &readySockets)) { // Existing client
				std::cout << "\033[1m[SERVER] [socket: " << i << "] Receiving from existing client\033[0m" << std::endl;
				memset(buff, 0, 4096);
				recvsize = recv(i, buff, 4096, 0);

				if (recvsize < 0 && errno != EWOULDBLOCK) {
					// if (i == maxFD)
					// 	maxFD -= 1;
					// FD_CLR(i, &currentSockets);
					printErrno(RECV, NO_EXIT);
					std::cerr << "\033[1m[SERVER] Error encountered while receiving message\033[0m" << std::endl;
					// close(i);
				}
				else if (recvsize == 0 && errno != EWOULDBLOCK) {
					std::cout << "\033[1m[SERVER] [socket: " << i << "] Client disconnected\033[0m" << std::endl;
					close(i);
					if (i == maxFD)
						maxFD -= 1;
					FD_CLR(i, &currentSockets);
				}
				else if (recvsize > 0) {
					std::cout << "\033[1m[SERVER] [socket: " << i << "] Receiving request:\033[0m" << std::endl;
					std::cout << std::string(buff) << std::endl;
                    request = HttpRequestParse::parse(std::string(buff), SockPortMap[i]);
					filepath = get_file_path(request, config, status);
					output = formatter.format_response(request, status, filepath, config);
					std::cout << output.c_str() << std::endl;
					send(i, output.c_str(), output.length(), 0);
				}
			}
    	}
	}
	std::cout << "\033[31;1mClosing all sockets:\033[0m";
	for (int i = 0; i < maxFD + 1; ++i) {
		std::cout << " " << i;
		close(i);
		FD_CLR(i, &currentSockets);
	}
	std::cout << std::endl;
	for (unsigned int i = 0; i < servsock.size(); ++i) {
		close(servsock[i]);
	}
    return(0);
}
