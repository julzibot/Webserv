/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/13 00:40:09 by julzibot         ###   ########.fr       */
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
        case EBADF: std::cerr << "errno: Bad file descriptor" << std::endl;
        break;
        case ECONNABORTED: std::cerr << "errno: Connection was aborted" << std::endl;
        break;
        case EINVAL: std::cerr << "errno: Invalid argument" << std::endl;
        break;
        case ENOTSOCK: std::cerr << "errno: Descriptor is not a socket" << std::endl;
        break;
        case EWOULDBLOCK: std::cerr << "errno: Non-blocking socket" << std::endl;
        break;
        case EADDRINUSE: std::cerr << "errno: Address already in use" << std::endl;
        break;
        case EINTR: std::cerr << "Interrupted system call" << std::endl;
        break;
        default: std::cerr << "errno: Unknown error code, should update: " << errno << std::endl;
    }

    if (ex)
        exit(errno);
}

int main (void)
{
    HttpRequest request;
    int         status;
    // SERVER
    Config  config = parse_config_file("conf_parsing/webserv.conf");
    int arrsize = config.get_portnums().size();
    std::vector<sockaddr_in> saddr(arrsize);
    for (int i = 0; i < arrsize; i++)
    {
        saddr[i].sin_family = AF_INET,
        saddr[i].sin_addr.s_addr = INADDR_ANY,
        saddr[i].sin_port = htons(config.get_portnums()[i]);
    }

    int option = 1;
    int servsock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(servsock, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
    if (servsock == -1)
    {
        std::cerr << "error encountered while trying to create socket !" << std::endl;
        return (-1);
    }

    // CLIENT
    struct sockaddr_in caddr;
    socklen_t caddrsize = sizeof(caddr);
    int clientsock;

    // BINDING
    setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    int ret = bind(servsock, (struct sockaddr*)&saddr[0], sizeof(saddr[0]));
    if (ret < 0) {
        printErrno(BIND, EXIT);
    }

    //LISTENING
    ret = listen(servsock, SOMAXCONN);
    if (ret < 0) {
        printErrno(LISTEN, EXIT);
    }
    std::cout << "[Server] listening on port " << config.get_portnums()[0] << std::endl;

    //WAITING TO ACCEPT
    char    buff[4096];
    std::string output;
    std::string filepath;
    std::ifstream fs;
    std::string line;
    std::string prevReqPath = "";
    ResponseFormatting  formatter;

    fd_set            currentSockets;
    fd_set            readySockets;
    int               maxFD = servsock;
    struct timeval    timeoutSocket;
    timeoutSocket.tv_usec = 1;
    timeoutSocket.tv_sec = 0;

    FD_ZERO(&currentSockets);
    FD_SET(servsock, &currentSockets);
    int recvsize;

    signal(SIGINT, sigHandler);
    while (isTrue)
    {
        status = 200;
		readySockets = currentSockets;
		if (select(maxFD + 1, &readySockets, NULL, NULL, &timeoutSocket) < 0)
			printErrno(SELECT, EXIT);
		for (int i = 0; i < maxFD + 1; ++i)
		{
			if (FD_ISSET(i, &readySockets))
			{
				if (i == servsock) // i is servSock, so accept new connection
				{
					std::cout << "\033[31mNow processing socket number: " << i << "\033[0m" << std::endl;
					clientsock = accept(servsock, (struct sockaddr*)&caddr, (socklen_t*)&caddrsize);
					if (clientsock < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
						printErrno(ACCEPT, NO_EXIT);
						// for (int j = 0; j < maxFD + 1; ++j)
						// 	if (FD_ISSET(j, &currentSockets))
						// 		close(i);
						// break;
					}
                    else if (clientsock >= 0)
                    {
                        fcntl(clientsock, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
                        if (clientsock > maxFD)
                            maxFD = clientsock;
                        FD_SET(clientsock, &currentSockets);
                    }
				}
				else { // clientsock
					// std::cout << "\033[31mServer socket:   " << servsock << "\033[0m" << std::endl;
					// std::cout << "\033[31mExisting client: " << i << "\033[0m" << std::endl;
					memset(buff, 0, 4096);
					recvsize = recv(i, buff, 4096, 0);

					if (recvsize < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
						printErrno(RECV, NO_EXIT);
						std::cerr << "\033[1m[SERVER] Error encountered while receiving message\033[0m" << std::endl;
						close(i);
					}
					else if (recvsize == 0) {
						std::cout << "\033[1m[SERVER] Client disconnected\033[0m" << std::endl;
						close(i);
						FD_CLR(i, &currentSockets);
					}
					else if (recvsize > 0) {
					std::cout << "\033[1m[SERVER] Client connected with success\033[0m" << std::endl;
					std::cout << std::string(buff) << std::endl;
				
					request = HttpRequestParse::parse(std::string(buff), config.get_portnums()[0]);
					filepath = get_file_path(request, config, status);
					output = formatter.format_response(request, status, filepath, config);
					std::cout << "output: " << output.c_str() << std::endl;
					send(i, output.c_str(), output.length(), 0);
					}
				}
			}
    	}
	}
	for (int i = 0; i < maxFD + 1; ++i) {
		if (FD_ISSET(i, &currentSockets))
			close(i);
	}
    close(servsock);
    return(0);
}
