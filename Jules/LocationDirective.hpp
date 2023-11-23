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
		bool					autoindex;
		std::string				server_name;
		std::string 			route;
		std::string 			root;
		std::vector<std::string> 	index;
		std::vector<METHODS>	methods_allowed;
		std::string				redirect_url;
		// std::string				alias;
		// std::vector<std::string>	try_files;

    public:
        // int get_root();
		std::string get_server_name();
		std::string get_route();
		std::vector<std::string> get_index();
		bool get_autoindex();
		std::vector<METHODS> get_methods_allowed();
		std::string get_redirect_url();
        std::string get_root() const;

		void	setRoute(std::string route);
		void	setRoot(std::string root);
		void	setindex(std::string indexFiles);
};

void	LocationDir::setRoute(std::string route)
{
	this->route = route;
}

void	LocationDir::setRoot(std::string root)
{
	this->root = root;
}

void	LocationDir::setindex(std::string indexFiles)
{
	std::istringstream line(indexFiles);
	std::string	buffer;

	while (line >> buffer)
		this->index.push_back(buffer);
}

std::string LocationDir::get_root() const
{
	return this->root;
}

std::vector<std::string> LocationDir::get_index()
{
	return this->index;
}
