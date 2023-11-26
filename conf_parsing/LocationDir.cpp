/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDir.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:43:37 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/26 19:29:06 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationDir.hpp"

// Milan 26/11
// 'auth_except' added
// Constructor defined
// removeMethod added

LocationDir::LocationDir( void ) {

	this->methods_allowed.push_back("GET");
	this->methods_allowed.push_back("POST");
	this->methods_allowed.push_back("DELETE");
}

void	LocationDir::removeMethod( std::string toRemove ) {

	std::vector<std::string>::iterator it;

	if (toRemove != "GET" && toRemove != "POST" && toRemove != "DELETE")
		throw (std::invalid_argument("Unknown method in 'auth_except'."));
	for (it = methods_allowed.begin(); it != methods_allowed.end(); ++it) {
		
		if (*it == toRemove)
		{
			methods_allowed.erase(it);
			return;
		}
		std::cout << "removeMethod loop" << std::endl;
	}
}

/* Accessors */
void	LocationDir::setindex(std::string indexFiles)
{
	std::istringstream line(indexFiles);
	std::string	buffer;

	while (line >> buffer)
		this->index.push_back(buffer);
}