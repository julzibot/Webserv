/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/06 16:26:34 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "RequestParsing.hpp"
#include "ResponseFormatting.hpp"
#include "conf_parsing/Config.hpp"
#include "conf_parsing/DirectiveParsing.h"

int main (void)
{
    // HttpRequestParse    parser;
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
    ResponseFormatting  formatter;
    int recvsize;
    int c = 0;
    while (true)
    {
        clientsock = accept(servsock, (struct sockaddr*)&caddr, (socklen_t*)&caddrsize);
        std::cout << "[Server] Client connected with success" << std::endl;
        while (strncmp(buff, strdup("end"), 3) != 0)
        {
            memset(buff, 0, 4096);
            recvsize = recv(clientsock, buff, 4096, 0);
            std::cout << std::string(buff) << std::endl;
            if (recvsize == -1)
                {std::cerr << "Error encountered receiving message"; break;} 
            else if (!recvsize)
                {std::cout << "Client disconnected" << std::endl; break;}
            HttpRequest request = HttpRequestParse::parse(std::string(buff), config.get_portnums()[0]);
            filepath = get_file_path(request, config);
            output = formatter.format_response("HTTP/1.1", 200, filepath, config);
            send(clientsock, output.c_str(), output.length(), 0);
        }
        close(clientsock);
        close(servsock);
        break;
    }
    return(0);
}