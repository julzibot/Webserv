/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:43:37 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/10 22:05:24 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

// Milan 27/11
// 'methods_except' added
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
		throw (std::invalid_argument("Unknown method in 'methods_except'."));
	for (it = methods_allowed.begin(); it != methods_allowed.end(); ++it) {

		if (*it == toRemove)
		{
			methods_allowed.erase(it);
			return;
		}
	}
}

/* Accessors */
void	LocationDir::setindex(std::string indexFiles)
{
	std::istringstream line(indexFiles);
	std::string	buffer;

	while (line >> buffer)
		this->index.push_back(std::string(buffer));
}

std::string	Config::get_type(std::string file_ext)
{
	return (this->types[file_ext]);
}
