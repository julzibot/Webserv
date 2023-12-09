/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/09 16:22:20 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "RequestParsing.hpp"
#include "ResponseFormatting.hpp"
#include "conf_parsing/Config.hpp"
#include "conf_parsing/DirectiveParsing.h"
#include <csignal>
#include <fcntl.h>

volatile sig_atomic_t    isTrue = 1;

// Function to run when CTRL-C is pressed
void    sigHandler( int param ) { isTrue = 0;}

void    printErrno( void )
{
    if (errno == EBADF)
        std::cerr << "errno: Bad file descriptor" << std::endl;
    else if (errno == ECONNABORTED)
        std::cerr << "errno: Connection was aborted" << std::endl;
    else if (errno == EINVAL)
        std::cerr << "errno: Invalid argument" << std::endl;
    else if (errno == ENOTSOCK)
        std::cerr << "errno: Descriptor is not a socket" << std::endl;
    else if (errno == EWOULDBLOCK || errno == EAGAIN)
        std::cerr << "errno: Non-blocking socket" << std::endl;
    else if (errno == EADDRINUSE)
        std::cerr << "errno: Address already in use" << std::endl;
    else
        std::cout << "errno: Unknown error code, should update: " << errno << std::endl;
}

int main (void)
{
    HttpRequest request;
    int         status;
    // SERVER
    Config  config = parse_config_file("conf_parsing/webserv.conf");
    int arrsize = config.get_portnums().size();
    struct sockaddr_in saddr[arrsize];
    for (int i = 0; i < arrsize; i++)
    {
        saddr[i].sin_family = AF_INET,
        saddr[i].sin_addr.s_addr = INADDR_ANY,
        saddr[i].sin_port = htons(config.get_portnums().at(i));
    }

    int option = 1;
    // int saddr_size = sizeof(saddr);
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
        std::cerr << "Bind(): ";
        printErrno();
        exit(errno);
    }

    //LISTENING
    ret = listen(servsock, SOMAXCONN);
    if (ret < 0) {
        std::cerr << "Listen(): ";
        printErrno();
        exit(errno);
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
    int recvsize;
    int c = 0;
    signal(SIGINT, sigHandler);
    while (isTrue)
    {
        clientsock = accept(servsock, (struct sockaddr*)&caddr, (socklen_t*)&caddrsize);
        if (clientsock < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cout << "Accept(): ";
            printErrno();
            break;
        }
        else if (clientsock >= 0)
        {
            status = 200;
            std::cout << "[Server] Client connected with success" << std::endl;
            memset(buff, 0, 4096);
            std::cout << "\e[31mRECV\e[0m" << std::endl;
            recvsize = recv(clientsock, buff, 4096, 0);
            if (recvsize < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
                std::cout << "Recv(): ";
                printErrno();
                std::cerr << "Error encountered while receiving message" << std::endl;
                break;
            }
            else if (recvsize == 0)
                std::cout << "[Server] Client disconnected" << std::endl;
            else if (recvsize > 0)
            {
                std::cout << std::string(buff) << std::endl;
                
                request = HttpRequestParse::parse(std::string(buff), config.get_portnums()[0]);
                filepath = get_file_path(request, config, prevReqPath, status);
                
                output = formatter.format_response("HTTP/1.1", status, filepath, config);
                std::cout << output << std::endl;
                send(clientsock, output.c_str(), output.length(), 0);
            }
            close(clientsock);
            std::cout << "[Server] Client socket closed" << std::endl;
        }
    }
    close(servsock);
    return(0);
}