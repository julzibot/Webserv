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

int main (void)
{
    // HttpRequestParse    parser;
    // SERVER
    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };

    // int option = 1;
    int saddr_size = sizeof(saddr);
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
    bind(servsock, (struct sockaddr*)&saddr, sizeof(saddr));

    //LISTENING
    listen(servsock, SOMAXCONN);
    std::cout << "[Server] listening on port " << PORT << std::endl;

    //WAITING TO ACCEPT
    char    buff[4096];
    std::string output;
    std::string filepath;
    std::ifstream fs;
    std::string line;
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
            if (recvsize == -1)
                {std::cerr << "Error encountered receiving message"; break;} 
            else if (!recvsize)
                {std::cout << "Client disconnected" << std::endl; break;}
            // PARSE THE REQUEST
            HttpRequest request = HttpRequestParse::parse(std::string(buff));
            request.port_number = PORT;
            // PARSE THE CONFIG FILE
            Config  config = parse_config_file("conf_parsing/webserv.conf");

            // BUILD THE RESPONSE,
            // FIRST BY GETTING THE FILE PATH, FILLING A RESPONSE OBJECT, THEN SENDING IT ALL AS A SINGLE STRING
            output += "HTTP/1.1 200 OK\n\n";
            filepath = get_file_path(request, config);
            fs = std::ifstream(filepath);
            while (std::getline(fs, line))
                output += line + '\n';
            std::cout << output << std::endl;
            send(clientsock, output.c_str(), output.length(), 0);
        }
        close(clientsock);
        close(servsock);
        break;
    }
    return(0);
}