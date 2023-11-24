/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationDir.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:43:37 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/24 15:50:27 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationDir.hpp"

/* Accessors */
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