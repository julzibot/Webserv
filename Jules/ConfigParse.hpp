#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>

class ConfigServer
{
    private:
        int port;
        std::string server_name;
        std::unordered_map<std::string, std::string> root;
        std::unordered_map<std::string, std::forward_list<std::string> > file;
    public:    
        int get_port();
        std::string get_servname() const;
        std::string get_root(std::string location) const;
        std::forward_list<std::string> get_files(std::string location) const;
};

class ConfigParse
{
    private:
        int worker_processes;
        int worker_connections;
        std::unordered_map<std::string, std::string> types;
        ConfigServer server;
    public:
        int get_workproc() const;
        int get_workco() const;
        std::string get_type(std::string file_ext) const;
        ConfigServer get_serv_info() const;
};