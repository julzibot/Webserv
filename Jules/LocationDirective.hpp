#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <vector>

class LocationDir
{
    private:
		std::string				server_name;
		std::string 			route;
		std::string 			root;
		std::vector<std::string> 	index;
		bool					autoindex;
		std::vector<std::string>	methods_allowed;
		std::string				redirect_url;
		// std::string				alias;
		// std::vector<std::string>	try_files;
        
    public:
        std::string	get_root();
		std::string get_server_name();
		std::string get_route();
		std::vector<std::string> get_index();
		bool get_autoindex();
		std::vector<std::string> get_methods_allowed();
		std::string get_redirect_url();
        std::string get_root() const;
};
