#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <vector>

enum METHODS { GET, POST, DELETE, PUT };

class LocationDir
{
    private:
		std::string				server_name;
		std::string 			route;
		std::string 			root;
		std::forward_list<std::string> 	index;
		bool					autoindex;
		std::vector<METHODS>	methods_allowed;
		std::string				redirect_url;
		// std::string				alias;
		// std::vector<std::string>	try_files;

    public:
        int get_root();
		std::string get_server_name();
		std::string get_route();
		std::forward_list<std::string> get_index();
		bool get_autoindex();
		std::vector<METHODS> get_methods_allowed();
		std::string get_redirect_url();
        std::string get_root() const;
};
