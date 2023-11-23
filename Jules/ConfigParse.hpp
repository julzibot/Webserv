#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <vector>

<<<<<<< HEAD
class ConfigServer
{
    private:
        int port;
        std::string server_name;
        std::unordered_map<std::string, std::string> root;
        std::unordered_map<std::string, std::forward_list<std::string> > file;
    public:    
        int								get_port();
        std::string 					get_servname() const;
        std::string						get_root(std::string location) const;
        std::forward_list<std::string>	get_files(std::string location) const;
};
=======
#include "RequestParsing.hpp"
#include "LocationDirective.hpp"
>>>>>>> a746cfa9176238c3ac4923ae1416edcb5f912e22

class ConfigParse
{
    private:
<<<<<<< HEAD
        int				worker_processes;
        int				worker_connections;
        ConfigServer	server;
        std::unordered_map<std::string, std::string>	types;
=======
        int worker_processes;
        int worker_connections;
        std::unordered_map<std::string, std::string> types;
		// std::unordered_map<int, std::vector<std::string> > loc_index;
		std::vector<int> servPortNums;
        std::unordered_map<int, std::unordered_map<std::string, LocationDir> > server;
        std::vector<int> error_codes;
        std::map<int, std::vector<std::string> > error_pages;
>>>>>>> a746cfa9176238c3ac4923ae1416edcb5f912e22
    public:
        int get_workproc() const { return (this->worker_processes); };
        int get_workco() const;
		std::unordered_map<int, std::unordered_map<std::string, LocationDir> > getServ() const;
        std::string get_type(std::string file_ext) const;
<<<<<<< HEAD
        ConfigServer get_serv_info() const;

		void	setWorkerCnts( int workerCnts ) { this->worker_connections = workerCnts; };
		void	setWorkerPrcs( int workerPrcs ) { this->worker_processes = workerPrcs; };

		// Unordered map
		std::unordered_map<std::string, std::string>::iterator	getBeginTypesMap( void ) { return (this->types.begin()); };
		std::unordered_map<std::string, std::string>::iterator	getEndTypesMap( void ) { return (this->types.end()); };
		void	addToTypes(std::string key, std::string value ) { this->types[key] = value; };
};
=======
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
>>>>>>> a746cfa9176238c3ac4923ae1416edcb5f912e22
