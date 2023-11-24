/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDirective.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 21:01:53 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/23 21:01:54 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <vector>

class LocationDir
{
    private:
		bool						autoindex;
		std::string					server_name;
		std::string 				route;
		std::string 				root;
		std::string					redirect_url;
		std::vector<std::string>	index;
		std::vector<std::string>	methods_allowed;
		// std::string				alias;
		// std::vector<std::string>	try_files;

    public:

		/* Accessors */
		bool						get_autoindex() const { return (this->autoindex); };
		std::string					get_server_name() const { return (this->server_name); };
		std::string					get_route() const { return (this->route); };
        std::string					get_root() const { return (this->root); };
		std::string					get_redirect_url() const { return (this->redirect_url); };
		std::vector<std::string>	get_index() const { return (this->index); };
		std::vector<std::string>		get_methods_allowed() const { return (this->methods_allowed); };

		void	setRoute(std::string route);
		void	setRoot(std::string root);
		void	setindex(std::string indexFiles);
};

void	LocationDir::setRoute(std::string route)
{
	this->route = route;
}

void	LocationDir::setRoot(std::string root)
{
	this->root = root;
}

void	LocationDir::setindex(std::string indexFiles)
{
	std::istringstream line(indexFiles);
	std::string	buffer;

	while (line >> buffer)
		this->index.push_back(buffer);
}