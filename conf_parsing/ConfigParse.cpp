/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:27:12 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/09 16:29:03 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "DirectiveParsing.h"

typedef std::map<std::string, std::string> strstrMap;

std::string parse_comments(std::string original_line)
{
	std::string line(original_line);
	size_t 		hashpos;
	size_t 		quotepos;
	size_t 		sq;
	size_t 		sec_quotepos;

	hashpos = line.find('#');
	quotepos = line.find('\"');
	sq = line.find('\'');
	quotepos = sq < quotepos ? sq : quotepos;
	sec_quotepos = 0;
	while (hashpos != NPOS)
	{
		if (quotepos != NPOS && quotepos < hashpos)
		{
			sec_quotepos = line.substr(quotepos + 1).find(line[quotepos]);
			if (sec_quotepos != NPOS)
				sec_quotepos += quotepos + 1;
			else
				throw std::exception();
		}
		if (quotepos > hashpos)
		{
			line = line.substr(0, hashpos);
			break;
		}
		else if (quotepos < hashpos && sec_quotepos < hashpos)
		{
			quotepos = line.substr(sec_quotepos + 1).find('\"');
			sq = line.substr(sec_quotepos + 1).find('\'');
			quotepos = sq < quotepos ? sq : quotepos;
			if (quotepos != NPOS) 
				quotepos += sec_quotepos + 1;
		}
		else
		{
			sq = hashpos;
			hashpos = line.substr(hashpos + 1).find('#');
			if (hashpos != NPOS)
				hashpos += sq + 1;
		}
	}
	return (line);
}

template <typename T>
void    expandInclude(std::string &line, T &s) 
{
    std::istringstream	toParse(line);
	std::string			str = "";
    std::string			command;
	std::string			filename;
    std::string			fileLine;
    std::string			fileContent;

	toParse >> command >> filename;
	if (command != "include")
		return;
    std::ifstream    fs(filename);
    if (fs.fail())
        throw (std::invalid_argument("Bad file/path."));
    while (std::getline(fs, fileLine)) {
        fileContent += fileLine;
        fileContent += "\n";
    }
    fs.close();
	while (std::getline(s, line))
		str += line + "\n";
	s.clear();
	s.str(fileContent + str);
	line = "";
	while (line.empty())
		std::getline(s, line);
}

size_t	isBrace(char brace, std::string line) {

	size_t	braceRes = line.find(brace);

	if (braceRes != NPOS) 
	{
		size_t	i = braceRes;
		size_t	len = line.length();
		while (++i < len && isspace(line[i]))
			;
		if (i < len)
			throw (std::invalid_argument("Characters found after brace."));
	}
	return (braceRes);
}

template <typename T>
void	get_braces_content(std::string dir_key, T &stream, std::map<std::string, std::string> &directives, std::vector<std::string> &dir_index)
{
	bool		add_portnum = 0;
	int			open_braces = 1;
	int			i = dir_index.size() - 1;
	std::string	line;
	std::string	portnum = "";

	dir_index.push_back(dir_key);
	while (open_braces && std::getline(stream, line))
	{
		line = parse_comments(line);
		if (add_portnum && line.find("listen") != NPOS)
		{
			std::istringstream(line) >> portnum >> portnum;
			portnum = " " + portnum;
			dir_index.push_back(dir_key + portnum);
			add_portnum = 0;
		}
		if (isBrace('{', line) != NPOS)
		{
			open_braces++;
			dir_key = line.substr(0, line.find('{'));
			if (dir_key.find("server") != NPOS)
				add_portnum = 1;
			if (!add_portnum)
				dir_index.push_back(dir_key + portnum);
		}
		else if (isBrace('}', line) != NPOS && --open_braces > 0)
			dir_key = dir_index.at(i + open_braces);
		else if (open_braces)
			directives[dir_key + portnum] += line + "\n";
	}
	if (open_braces)
		throw std::invalid_argument("Unclosed braces found.");
}

Config	parse_config_file(std::string path)
{
    Config						config;
	int							i = 0;
    size_t						bracepos;
    std::ifstream				conf_file(path);
    std::istringstream			ls;
	strstrMap					directives;
    std::string					line;
	std::string					buffer;
    std::string					directive = "main";
	std::vector<std::string>	dir_index;

	while (std::getline(conf_file, buffer))
		line += buffer + '\n';
	ls.str(line);
	conf_file.close();

    while (std::getline(ls, line) || i < dir_index.size())
    {
		if (ls.eof() && i < dir_index.size())
		{
			directive = dir_index.at(i);
			ls.clear();
			ls.str(directives[dir_index.at(i)]);
			std::getline(ls, line);
			i++;
		}
		line = parse_comments(line);
		if (line.find("include") != NPOS)
            expandInclude(line, ls);
		bracepos = isBrace('{', line);
		if (bracepos != NPOS)
			get_braces_content<std::istringstream>(line.substr(0, bracepos), ls, directives, dir_index);
		parseDirective(line, directive, config);
    }

	// config.printAll();
	
	// TESTING PARSING OUTPUT
	// for (i = 0; i < dir_index.size(); i++)
	// 	std::cout << "\e[31mkey: \e[0m" << dir_index.at(i) << "  \e[33mvalue: \e[0m"
	// 		<< directives[dir_index.at(i)] << std::endl << "\e[34m----------\e[0m" << std::endl;
	return (config);
}

	// std::cout << "| " << config.getLocRef(request.port_number, "/").get_root() << " |" << std::endl;
	/*
	 * 0. Check the port_number to get the required locations vector.
	 * 1. Check the request path.
	 * 1.5: Check if the METHOD matches for this path
	 * 2. Check the location.
	 * 3. Test for file mentioned in index or 
	 * 		one obtained by appending the path name.
	 * 4. Use the try files directive to find the file.
	 * 5. If file is found, return the path.
	 * 6. Check if directory listing is ON
	*/

std::string get_file_path(HttpRequest &request, Config &config, std::string &prevPath, int &status_code)
{
	std::string file_path;
	unsigned int	i = 0;
	std::map<std::string, LocationDir>	&locations = config.getLocMap(request.port_number);
	std::map<std::string, LocationDir>::iterator	it = locations.begin();
	std::map<std::string, LocationDir>::iterator	locEnd = locations.end();
	std::vector<std::string> ind;
	std::string	locRoute;
	int	slashPos = 1;

	if (request.path.find('.') != NPOS)
	{
		HttpRequest	newReq = HttpRequest(request);
		newReq.prio_file = newReq.path.substr(1);
		newReq.path = prevPath;
		return (get_file_path(newReq, config, prevPath, status_code));
	}
	while (it != locEnd)
	{
		locRoute = it->first;
		if (request.path.length() > 1)
			slashPos = request.path.find('/', 1);
		if (locRoute == request.path.substr(0, slashPos))
		{
			if (!(it->second.get_redir().empty()))
			{
				status_code = 301;
				return (it->second.get_redir());
			}
			prevPath = locRoute;
			break;
		}
		it++;
	}
	if (it == locEnd && locations.begin() != locEnd)
		std::cout << "problem encountered" << std::endl;//error handling
	else
	{
		std::vector<std::string> methods = it->second.get_methods_allowed();
		while (i < methods.size() && methods.at(i) != request.method)
			i++;
		if (i < methods.size())
		{
			ind = it->second.get_index();
			if (!request.prio_file.empty())
				ind.insert(ind.begin(), request.prio_file);
			for (int j = 0; j < ind.size(); j++)
			{
				file_path = it->second.get_root() + request.path;
				if (request.path.length() > 1) file_path += '/';
				file_path += ind[j];
				if (!access(file_path.c_str(), R_OK))
					return (file_path);
			}
			status_code = 404;
			return ("");
			// directory listing;
		}
		status_code = 404;
		return ("");
	}
	status_code = 404;
	return ("");
	// If code reaches this section, the route and method do not exist.
	// TODO: Throw an error for route/path not existing.
}

// std::string	Config::get_error_page_file_path(int code, Config &config,
// 		int port) const
// {
// 	HttpRequest request;
// 	std::string	file_path;

// 	request.port_number = port;
// 	request.method = "GET";
// 	request.path = config.get_route_for_error_code(code, port);
// 	file_path = get_file_path(request, config);
// 	return (file_path);
// }

// int main()
// {
// 	std::string path = "webserv.conf";
	// std::ifstream file(path);

// 	Config c = parse_config_file(path);
// 	return (0);
// }

void	Config::printAll( void ) {

	std::cout << "worker_processes: " << this->worker_processes << std::endl;
	std::cout << "worker_connections: " << this->worker_connections << std::endl;
	
	std::cout << "\e[31m************************\e[0m" << std::endl;
	std::cout << "\e[31m*** SERVER LOCATIONS ***\e[0m" << std::endl;
	for (servLocMap::iterator it = this->server.begin(); it != this->server.end(); ++it) {

		std::cout << "\e[4;32mPORT: " << it->first << "\e[0m" << std::endl;
		std::map<std::string, LocationDir>::iterator it1;
		for (it1 = it->second.begin(); it1 != it->second.end(); ++it1) {
			std::cout << "\e[35m* LocationDir: " << it1->first << " *\e[0m" << std::endl;
			std::cout << "\e[33mAutoindex:    \e[0m" << it1->second.get_autoindex() << std::endl;
			std::cout << "\e[33mServer_name:  \e[0m" << it1->second.get_server_name() << std::endl;
			std::cout << "\e[33mRoute:        \e[0m" << it1->second.get_route() << std::endl;
			std::cout << "\e[33mRoot:         \e[0m" << it1->second.get_root() << std::endl;
			std::cout << "\e[33mRedirect_url: \e[0m" << it1->second.get_redirect_url() << std::endl;
			std::vector<std::string>	indexVec = it1->second.get_index();
			std::vector<std::string>::iterator	vecit;
			std::cout << "INDEX: ";
			for (vecit = indexVec.begin(); vecit != indexVec.end(); ++vecit)
				std::cout << *vecit << " ";
			std::cout << std::endl;
			std::vector<std::string>	methVec = it1->second.get_methods_allowed();
				std::cout << "METHODS: ";
			for (vecit = methVec.begin(); vecit != methVec.end(); ++vecit)
				std::cout << *vecit << " ";
			std::cout << std::endl;
		}
	}
	std::cout << "\e[31m************************\e[0m" << std::endl;
		std::cout << "\e[35mServer Port Numbers\e[0m" << std::endl;
		std::vector<int>::iterator itint;
		for (itint = servPortNums.begin(); itint != servPortNums.end(); ++itint)
			std::cout << *itint << " ";
		std::cout << std::endl;
		std::cout << "\e[36mError codes\e[0m" << std::endl;
		for (itint = error_codes.begin(); itint != error_codes.end(); ++itint)
			std::cout << *itint << " ";
		std::cout << std::endl;

		servErrorMap::iterator	errmapit;
		for (errmapit = error_page_map.begin(); errmapit != error_page_map.end(); ++errmapit) {

			std::cout << "\e[38mPORT: " << errmapit->first << "\e[0m" << std::endl;
			std::map<int, std::string>::iterator	mapintit;
			std::cout << "\e[37m* Error pages *\e[0m" << std::endl;
			for (mapintit = errmapit->second.begin(); mapintit != errmapit->second.end(); ++mapintit)
				std::cout << mapintit->first << " - " << mapintit->second << std::endl;
			std::cout << std::endl;
		}
}
