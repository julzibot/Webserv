/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 19:15:02 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/23 21:02:09 by mstojilj         ###   ########.fr       */
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
#include "../RequestParsing.hpp"
#include "LocationDirective.hpp"
# define NPOS std::string::npos

typedef std::unordered_map<int, std::unordered_map<std::string, LocationDir> > servLocMap;
typedef std::unordered_map<std::string, std::string> strstrMap;

class ConfigParse
{
    private:

        int											worker_processes;
        int											worker_connections;
        servLocMap									server;
        strstrMap									types;
		std::vector<int>							servPortNums;
        std::vector<int>							error_codes;
        std::map<int, std::vector<std::string> >	error_pages;
		// std::unordered_map<int, std::vector<std::string> > loc_index;

    public:
		/* Accessors */
        int				get_workproc() const { return (this->worker_processes); };
        int				get_workco() const { return (this->worker_connections); };
		servLocMap		getServ() const { return (this->server); };
        std::string		get_type(std::string file_ext);
        std::string		get_file_path(HttpRequest request) const;
        LocationDir		&getLocRef(int	port, std::string route) { return (this->server[port][route]); };
		std::unordered_map<std::string, LocationDir>	&getLocMap(int port);

        void	set_workproc(int value);
        void	set_workco(int value);
        void	add_type(std::string extension, std::string path);
};

std::unordered_map<std::string, LocationDir>	&ConfigParse::getLocMap(int port)
{
	return (this->server[port]);
}