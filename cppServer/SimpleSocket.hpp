/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/21 19:43:15 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/21 20:15:48 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMPLESOCKET_HPP
# define SIMPLESOCKET_HPP

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>

namespace	WebServ {
	
class SimpleSocket {

private:

	struct sockaddr_in	address;
	int					sock;
	int					connection;

public:

	SimpleSocket( int domain, int service, int protocol,
	int port, unsigned long interface );

	virtual int	connectToNetwork( int sock, struct sockaddr_in address ) = 0;
	void		testConnection( int );

	int	getSock( void ) { return (this->sock); };
	int	getConnection( void ) { return (this->connection); };
};
}

#endif