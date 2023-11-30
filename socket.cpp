/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/28 15:37:31 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "RequestParsing.hpp"
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
    int recvsize;
    int c = 0;
    signal(SIGINT, sigHandler);
    while (isTrue)
    {
        clientsock = accept(servsock, (struct sockaddr*)&caddr, (socklen_t*)&caddrsize);
        std::cout << "[Server] Client connected with success" << std::endl;
        memset(buff, 0, 4096);
        recvsize = recv(clientsock, buff, 4096, 0);
        std::cout << std::string(buff) << std::endl;
        if (recvsize == -1)
            {std::cerr << "Error encountered receiving message"; break;}
        else if (!recvsize)
            {std::cout << "Client disconnected" << std::endl; break;}
        // PARSE THE REQUEST
        request = HttpRequestParse::parse(std::string(buff), config.get_portnums()[0]);
        // BUILD THE RESPONSE,
        // FIRST BY GETTING THE FILE PATH, FILLING A RESPONSE OBJECT, THEN SENDING IT ALL AS A SINGLE STRING
        output += "HTTP/1.1 200 OK\n\n";
        filepath = get_file_path(request, config);
        fs = std::ifstream(filepath);
        while (std::getline(fs, line))
            output += line + '\n';
        std::cout << output << std::endl;
        send(clientsock, output.c_str(), output.length(), 0);
        output.clear();
        close(clientsock);
        std::cout<< isTrue << std::endl;
    }
    // signal(SIGINT, SIG_DFL);
    close(servsock);
    return(0);
}