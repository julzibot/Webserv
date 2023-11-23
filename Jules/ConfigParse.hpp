#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <vector>

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

class ConfigParse
{
    private:
        int				worker_processes;
        int				worker_connections;
        ConfigServer	server;
        std::unordered_map<std::string, std::string>	types;
    public:
        int get_workproc() const { return (this->worker_processes); };
        int get_workco() const;
        std::string get_type(std::string file_ext) const;
        ConfigServer get_serv_info() const;

		void	setWorkerCnts( int workerCnts ) { this->worker_connections = workerCnts; };
		void	setWorkerPrcs( int workerPrcs ) { this->worker_processes = workerPrcs; };

		// Unordered map
		std::unordered_map<std::string, std::string>::iterator	getBeginTypesMap( void ) { return (this->types.begin()); };
		std::unordered_map<std::string, std::string>::iterator	getEndTypesMap( void ) { return (this->types.end()); };
		void	addToTypes(std::string key, std::string value ) { this->types[key] = value; };
};