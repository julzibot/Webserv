/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DELETE.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/21 16:16:58 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/21 18:00:49 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

void	WebServ::deleteResource( const std::string& resource ) {

	std::string p = _request.path.substr(0, _request.path.find('/', 1));
	std::string	root = _config.getServMain(_request.port_number, p, true)["root"];
	if (root.empty()) {
		std::cerr << "deleteResource: 'root' not found" << std::endl;
		_status = 500; // 500 Internal Server Error
		return;
	}

	std::string	pathToResource = root + resource;
	struct stat	fileInfos;
	if (stat(pathToResource.c_str(), &fileInfos) < 0) {
		_status = 404; // Resource not found (doesn't exist)
		return;
	}
	if (std::remove(pathToResource.c_str()) < 0) {
		_status = 500;
		std::cerr << "Error: deleteResource(): Couldn't delete file" << std::endl;
	}
	_status = 204;
}
