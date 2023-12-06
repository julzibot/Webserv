/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/06 19:20:02 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "RequestParsing.hpp"
#include "ResponseFormatting.hpp"
#include "conf_parsing/Config.hpp"
#include "conf_parsing/DirectiveParsing.h"
#include <csignal>

volatile sig_atomic_t    isTrue = 1;

// Function to run when CTRL-C is pressed
void    sigHandler( int param ) {isTrue = 0;}

int main (void)
{
    HttpRequest request;
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
    // setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option));
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
    bind(servsock, (struct sockaddr*)&saddr[0], sizeof(saddr[0]));

    //LISTENING
    listen(servsock, SOMAXCONN);
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
        std::cout << "[Server] Client connected with success" << std::endl;
        memset(buff, 0, 4096);
        recvsize = recv(clientsock, buff, 4096, 0);
        if (recvsize == -1)
            {std::cerr << "Error encountered receiving message"; break;} 
        else if (!recvsize)
            {std::cout << "Client disconnected" << std::endl; break;}
        std::cout << std::string(buff) << std::endl;
        request = HttpRequestParse::parse(std::string(buff), config.get_portnums()[0]);
        filepath = get_file_path(request, config, prevReqPath);
        output = formatter.format_response("HTTP/1.1", 200, filepath, config);
        std::cout << "output: " << output.c_str() << std::endl;
        send(clientsock, output.c_str(), output.length(), 0);
        close(clientsock);
    }
    close(servsock);
    return(0);
}