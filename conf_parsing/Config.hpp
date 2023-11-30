/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 19:15:02 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/30 15:19:21 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <forward_list>
#include <vector>
#include "../RequestParsing.hpp"
#include "LocationDir.hpp"

# define NPOS std::string::npos

typedef std::map<int, std::map<std::string, LocationDir> > servLocMap;
typedef std::map<std::string, std::string> strstrMap;

class Config
{
    private:

        int											worker_processes;
        int											worker_connections;
        servLocMap									server;
        strstrMap									types;
		std::vector<int>							servPortNums;
        std::vector<int>							error_codes;
        std::map<int, std::vector<std::string>>	    error_pages;
        std::map<int, std::map<int, std::string>>   error_page_map;
		// std::map<int, std::vector<std::string> > loc_index;

    public:
		/* Accessors */
        int				get_workproc() const { return (this->worker_processes); };
        int				get_workco() const { return (this->worker_connections); };
		servLocMap		getServ() const { return (this->server); };
        std::string		get_type(std::string file_ext);
        std::string		get_file_path(HttpRequest request) const;
        LocationDir		&getLocRef(int	port, std::string route) { return (this->server[port][route]); };
		std::map<std::string, LocationDir>	&getLocMap(int port) { return (this->server[port]); };
        std::string	Config::get_error_page_file_path(int code, Config &config, int port) const;
        std::string     get_route_for_error_code(int code, int port) const {
            return (this->error_page_map.at(port).at(code));
        };

        void	set_workproc(int value) { this->worker_processes = value; };
        void	set_workco(int value) { this->worker_connections = value; };
        void	add_type(std::string extension, std::string path) { this->types[extension] += path; };
};
