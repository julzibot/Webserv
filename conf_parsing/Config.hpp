/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 19:15:02 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/22 18:53:53 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include "../RequestParsing.hpp"

class Config;
class LocationDir;

#include "DirectiveParsing.h"

# define NPOS std::string::npos

typedef std::map<std::string, std::string> strstrMap;
typedef std::map<std::string, std::map<std::string, LocationDir> > servLocMap;
typedef std::map<std::string, std::map<std::string, strstrMap> > servInfos;

class LocationDir
{
    private:
		bool						autoindex;
		std::string					server_name;
		std::string 				route;
		std::string					redir;
		std::string 				root;
		std::string					redirect_url;
		std::vector<std::string>	index;
		std::vector<std::string>	methods_allowed;

    public:
		LocationDir(void);
		void	removeMethod( std::string toRemove );

		/* Accessors */
		bool						get_autoindex() const { return (this->autoindex); };
		std::string					get_server_name() const { return (this->server_name); };
		std::string					get_route() const { return (this->route); };
		std::string					get_redir() const { return (this->redir); };
        std::string					get_root() const { return (this->root); };
		std::string					get_redirect_url() const { return (this->redirect_url); };
		std::vector<std::string>	get_index() const { return (this->index); };
		std::vector<std::string>	get_methods_allowed() const { return (this->methods_allowed); };

		void	setName(std::string name) { this->server_name = name; };
		void	setRoute(std::string route) { this->route = route; };
		void	setRedir(std::string redir) { this->redir = redir; };
		void	setRoot(std::string root) { this->root = root; };
		void	setAutoindex(bool boolean) { this->autoindex = boolean; };
		void	setindex(std::string indexFiles);
};

class Config
{
    private:

        int					worker_processes;
        int					worker_connections;
        strstrMap			types;
		strstrMap			cgi;
		strstrMap			hosts;
        servLocMap			server_locs;
        servInfos			server_main;
		std::vector<int>	servPortNums;

    public:
		/* Accessors */
        int					get_workproc() const { return (this->worker_processes); };
        int					get_workco() const { return (this->worker_connections); };
        std::vector<int>    get_portnums() const { return (this->servPortNums); };
		servLocMap			getServ() const { return (this->server_locs); };
        std::string			get_type(std::string file_ext);
        std::string			get_cgi_type(std::string file_ext);
        strstrMap			get_hostMap(void) { return(this->hosts); };
        LocationDir			&getLocRef(std::string const &hostIP, int port, std::string route)
							{ return (this->server_locs[hostIP + ":" + std::to_string(port)][route]); };
		std::map<std::string, LocationDir>&	getLocMap(std::string const &hostIP, int port)
							{ return (this->server_locs[hostIP + ":" + std::to_string(port)]); };
		bool				checkNullID(std::string const &tempHost, int const &port) {
								for (servLocMap::iterator it = this->server_locs.begin(); it != this->server_locs.end(); it++)
									if (it->first == tempHost + ":" + std::to_string(port))
										return (true);
									return (false);
							}
		strstrMap			&getServMain(std::string const &hostIP, int port, std::string const &route, bool const &init);

        void	set_workproc(int value) { this->worker_processes = value; };
        void	set_workco(int value) { this->worker_connections = value; };
        void	add_type(std::string extension, std::string path) { this->types[extension] += path; };
        void	add_cgi(std::string extension, std::string path) { this->cgi[extension] += path; };
        void	add_hosts(std::string hostname, std::string ip) { this->hosts[hostname] += ip; };
        void    add_portnum(int portnum) { this->servPortNums.push_back(portnum); };

		void	printAll( void );
};

Config	parse_config_file(std::string path);
std::string get_file_path(HttpRequest &request, Config &config, int &status);
LocationDir& get_Location_for_Path(HttpRequest const &request, Config &config);
