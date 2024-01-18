/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:43:37 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/11 00:25:49 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

LocationDir::LocationDir( void )
{
	this->methods_allowed.push_back("GET");
	this->methods_allowed.push_back("POST");
	this->methods_allowed.push_back("DELETE");
}

void	LocationDir::removeMethod( std::string toRemove )
{
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

std::string	Config::get_cgi_type(std::string file_ext)
{
	return (this->cgi[file_ext]);
}

strstrMap		&Config::getServMain(int port, std::string const  &route, bool init)
{
	std::map<std::string, strstrMap> &servers = this->server_main[port];
	std::map<std::string, strstrMap>::iterator it;

	if (!init)
		return (servers[route]);
	if (route.find('.') ==  NPOS)
	{
		for (it = servers.begin(); it != servers.end(); it++)
		{
			if (it->first == route)
				return (servers[route]);
		}
		return (servers["main"]);
	}
	std::string path;
	for (it = servers.begin(); it != servers.end(); it++)
	{
		path = it->second["root"] + route;
		if (!access(path.c_str(), F_OK))
			return (it->second);
	}
	return (servers["main"]);
}
