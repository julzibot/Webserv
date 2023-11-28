#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <vector>
#include <algorithm>

#include "RequestParsing.hpp"
#include "LocationDirective.hpp"

class ConfigParse
{
    private:
        int worker_processes;
        int worker_connections;
        std::unordered_map<std::string, std::string> types;
        std::map<int, std::vector<LocationDir>> server;
        std::vector<int> error_codes;
        std::map<int, std::vector<std::string>> error_pages;
    public:
        int get_workproc() const;
        int get_workco() const;
        std::string get_type(std::string file_ext) const;
        std::string get_file_path(HttpRequest request) const;
};
