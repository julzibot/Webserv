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
#include <unistd.h>
#include <forward_list>
#include <vector>
#include "../RequestParsing.hpp"
#include "DirectiveParsing.h"

# define NPOS std::string::npos

class LocationDir;

typedef std::map<int, std::map<std::string, LocationDir> > servLocMap;
typedef std::map<std::string, std::string> strstrMap;

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

		LocationDir( void );

		void	removeMethod( std::string toRemove );

		/* Accessors */
		bool						get_autoindex() const { return (this->autoindex); };
		std::string					get_server_name() const { return (this->server_name); };
		std::string					get_route() const { return (this->route); };
        std::string					get_root() const { return (this->root); };
		std::string					get_redirect_url() const { return (this->redirect_url); };
		std::vector<std::string>	get_index() const { return (this->index); };
		std::vector<std::string>	get_methods_allowed() const { return (this->methods_allowed); };

		void	setRoute(std::string route) { this->route = route; };
		void	setRoot(std::string root) { this->root = root; };
		void	setAutoindex(bool boolean) { this->autoindex = boolean; };
		void	setindex(std::string indexFiles);
};

class Config
{
    private:

        int											worker_processes;
        int											worker_connections;
		std::vector<int>							servPortNums;
        servLocMap									server;
        strstrMap									types;
        std::vector<int>							error_codes;
        std::map<int, std::vector<std::string> >	    error_pages;
        std::map<int, std::map<int, std::string> >   error_page_map;
		// std::map<int, std::vector<std::string> > loc_index;

    public:
		/* Accessors */
        int				get_workproc() const { return (this->worker_processes); };
        int				get_workco() const { return (this->worker_connections); };
        std::vector<int>    get_portnums() const { return (this->servPortNums); };
		servLocMap		getServ() const { return (this->server); };
        std::string		get_type(std::string file_ext) { return (this->types[file_ext]); };
        LocationDir		&getLocRef(int	port, std::string route) { return (this->server[port][route]); };
		std::map<std::string, LocationDir>	&getLocMap(int port) { return (this->server[port]); };
        std::string     get_error_page_file_path(int code, Config &config, int port) const;
        std::string     get_route_for_error_code(int code, int port) const 
            { return (this->error_page_map.at(port).at(code)); };

        void	set_workproc(int value) { this->worker_processes = value; };
        void	set_workco(int value) { this->worker_connections = value; };
        void	add_type(std::string extension, std::string path) { this->types[extension] += path; };
};