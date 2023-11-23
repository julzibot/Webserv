/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/21 19:47:26 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/21 20:14:28 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SimpleSocket.hpp"

WebServ::SimpleSocket::SimpleSocket( int domain, int service, int protocol,
	int port, unsigned long interface ) {

	// Define address structure
	// Domain - IP Address Family
	address.sin_family = domain;
	address.sin_port = htons(port);
	// Interface on which the port is running
	address.sin_addr.s_addr = htonl(interface);
	// Establish connection
	this->sock = socket(domain, service, protocol);
	testConnection(sock);
	this->connection = connectToNetwork(sock, address);
	testConnection(connection);
}

void	WebServ::SimpleSocket::testConnection( int itemToTest ) {

	if (itemToTest < 0) {

		std::cerr << "Failed to connect..." << std::endl;
		exit(EXIT_FAILURE);
	}
}