#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <vector>

#include "RequestParsing.hpp"
#include "LocationDirective.hpp"
# define NPOS std::string::npos

class ConfigParse
{
    private:
        int worker_processes;
        int worker_connections;
        std::unordered_map<std::string, std::string> types;
		// std::unordered_map<int, std::vector<std::string> > loc_index;
		std::vector<int> servPortNums;
        std::unordered_map<int, std::unordered_map<std::string, LocationDir> > server;
        std::vector<int> error_codes;
        std::map<int, std::vector<std::string> > error_pages;
    public:
        int get_workproc() const;
        int get_workco() const;
		std::unordered_map<int, std::unordered_map<std::string, LocationDir> > getServ() const;
        std::string get_type(std::string file_ext) const;
        std::string get_file_path(HttpRequest request) const;

        void    set_workproc(int value);
        void    set_workco(int value);
        void    add_type(std::string extension, std::string path);
        LocationDir	&getLocRef(int	port, std::string route);
		std::unordered_map<std::string, LocationDir>	&getLocMap(int	port);
};

LocationDir	&ConfigParse::getLocRef(int port, std::string route)
{
	return (this->server[port][route]);
}

std::unordered_map<std::string, LocationDir>	&ConfigParse::getLocMap(int port)
{
	return (this->server[port]);
}

std::unordered_map<int, std::unordered_map<std::string, LocationDir> > ConfigParse::getServ() const
{
	return(this->server);
}