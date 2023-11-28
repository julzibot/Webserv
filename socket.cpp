/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/24 22:39:28 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "RequestParsing.hpp"

int main (void)
{
    HttpRequestParse    parser;
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
    char    *output;
    int recvsize;
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

            output = parser.process_request(buff,recvsize, PORT);
            send(clientsock, output, recvsize + 1, 0);
        }
        close(clientsock);
        close(servsock);
        break;
    }
    return(0);
}