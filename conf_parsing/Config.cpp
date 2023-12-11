/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:43:37 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/11 15:49:51 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <dirent.h>
#include "../types/types.h"

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

std::string	LocationDir::get_directory_listing(HttpRequest &request, Config &config) const {
	DIR *dir;
	struct dirent *en;
	std::vector<std::string> list;
	std::string	output;

	std::cout << "This root is " << this->get_root() << std::endl;

	dir = opendir(this->get_root().c_str()); //open all or present directory
	if (dir) {
		std::cout << "Directory listing: " << std::endl;
		while ((en = readdir(dir)) != NULL) {
			list.push_back(en->d_name);
			std::cout << en->d_name << std::endl; //print all files in directory
		}
		closedir(dir); //close directory
		output = START_OF_LIST;
		output += "<h1>Directory listing for " + this->get_route() + "</h1>";
		output += "<ul>";
		for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); ++it) {
			std::cout << "File: " << *it << std::endl;
			output += "<li><a href=\"file://" + this->get_root() + "/" + *it + "\">" + *it + "</a></li>";
		}
		output += "</ul>";
		output += END_OF_LIST;
		return (output);
	} else {
		// TODO: Handle error case in case directory does not open.
		std::cout << "Could not open directory" << std::endl;
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
