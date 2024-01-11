/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:27:12 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/11 16:57:38 by julzibot         ###   ########.fr       */
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
	std::cout << filename << std::endl;
    std::ifstream    fs("./conf_parsing/" + filename);
    if (fs.fail())
        throw (std::invalid_argument("Config file: Bad include filename."));
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

size_t	isBrace(char brace, std::string line)
{
	size_t	braceRes = line.find(brace);

	if (braceRes != NPOS) 
	{
		unsigned int i;
		size_t	len = line.length();
		for (i = 0; i < len; i++)
		{
			if (brace == '{' && ((i > braceRes && !isspace(line[i])) \
					|| removeSpaces(line.substr(0, braceRes)).empty())) break;
			else if (brace == '}' && i != braceRes && !isspace(line[i])) break;
		}
		if (i < len)
			throw (std::invalid_argument("Config file: incorrect brace position."));
	}
	return (braceRes);
}

template <typename T>
void	get_braces_content(std::string dir_key, T &stream, std::map<std::string, std::string> &directives, std::vector<std::string> &dir_index)
{
	bool		add_portnum = 0;
	int			open_braces = 1;
	bool		can_write = true;
	std::string	line;
	std::istringstream	portline;
	std::string portbuff;
	std::string	portnum = "";
	std::vector<std::string>::iterator it;

	dir_index.push_back(dir_key);
	while (open_braces && std::getline(stream, line))
	{
		line = parse_comments(line);
		if (add_portnum && line.find("listen") != NPOS)
		{
			portnum = "";
			portline.str(line); portline >> portbuff;
			while (portline >> portbuff)
				portnum += " " + portbuff;
			it = std::find(dir_index.begin(), dir_index.end(), dir_key + portnum);
			while (it != dir_index.end())
			{
				portnum += "_";
				it = std::find(dir_index.begin(), dir_index.end(), dir_key + portnum);
			}
			dir_index.push_back(dir_key + portnum);
			portline.clear();
			add_portnum = 0;
		}
		else if (add_portnum && line.find("listen") == NPOS)
			throw std::invalid_argument("Config file: directive following server is not 'listen'");
		else if (!add_portnum && line.find("listen") != NPOS)
			throw std::invalid_argument("Config file: 'listen' directive at wrong line");
		if (isBrace('{', line) != NPOS)
		{
			open_braces++;
			can_write = true;
			dir_key = line.substr(0, line.find('{'));
			if (dir_key.find("server") != NPOS)
				add_portnum = 1;
			if (!add_portnum)
				dir_index.push_back(dir_key + portnum);
		}
		else if (isBrace('}', line) != NPOS)
		{
			open_braces--; can_write = false;
		}
		else if (open_braces && !can_write && !removeSpaces(line).empty())
			throw std::invalid_argument("Config file: \"" + removeSpaces(line) + "\": wrong line place");
		else if (open_braces)
			directives[dir_key + portnum] += line + "\n";
	}
	if (open_braces)
		throw std::invalid_argument("Config file: Unclosed braces found.");
}

Config	parse_config_file( std::string path )
{

    Config						config;
	unsigned int				i = 0;
    size_t						bracepos;
    std::istringstream			ls;
	strstrMap					directives;
    std::string					line;
	std::string					buffer;
    std::string					directive = "main";
	std::vector<std::string>	dir_index;
    std::ifstream				conf_file(path);
	
	if (!conf_file.good())
		throw std::invalid_argument("Invalid config file name");
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
		// std::cout << line << std::endl;
		parseDirective(line, directive, config);
	}

	// TESTING PARSING OUTPUT
	// for (i = 0; i < dir_index.size(); i++)
	// 	std::cout << "key: " << dir_index.at(i) << " value: " \
	// 		<< directives[dir_index.at(i)] << std::endl << "----------" << std::endl;
	// strstrMap infos;
	// std::vector<int> ports = config.get_portnums();
	// for (i = 0; i < ports.size(); i++)
	// {
	// 	std::cout << "PORT NUMBER " << ports[i] << std::endl;
	// 	infos = config.getServMain(ports[i]);
	// 	std::cout << "name: " << infos["server_name"] << " | root: "\
	// 		<< infos["root"] << " | error path: " << infos["error_pages"] << std::endl << "----------" << std::endl;
	// }
	return (config);
}

LocationDir& get_Location_for_Path(HttpRequest const &request, Config &config)
{
	std::map<std::string, LocationDir>	&locations = config.getLocMap(request.port_number);
	std::map<std::string, LocationDir>::iterator	it = locations.begin();
	std::map<std::string, LocationDir>::iterator	locEnd = locations.end();
	std::string	locRoute;

	while (it != locEnd)
	{
		locRoute = it->first;
		if (locRoute == request.path)
			return (it->second);
		++it;
	}
	return (it->second);
}

std::string	file_request_case(size_t const &slashPos, size_t &dotPos, HttpRequest &request, Config &config, int &status_code)
{
	std::string file_path;
	int	acss;

	if (dotPos < slashPos)
	{
		std::string p = request.path.substr(0, slashPos);
		file_path = config.getServMain(request.port_number, p, true)["root"] + request.path;
		acss = access(file_path.c_str(), F_OK);
		if (!acss && !access(file_path.c_str(), R_OK))
			return (file_path);
		else if (!acss)
			status_code = 403;
		else
			status_code = 404;
		return ("");
	}
	else
	{
		while (request.path[--dotPos] != '/') ;
		request.prio_file = request.path.substr(dotPos + 1);
		request.path = request.path.substr(0, dotPos);
		return ("");
	}
}

std::string	check_index_files(HttpRequest &request, std::map<std::string, LocationDir>::iterator it, size_t const &slashPos, size_t const &dotPos, int &status_code)
{
	std::vector<std::string> ind;
	std::string	file_path;
	int acss;

	ind = it->second.get_index();
	if (!request.prio_file.empty())
		ind.insert(ind.begin(), request.prio_file);
	file_path = it->second.get_root();
	if (file_path[file_path.length() - 1] != '/')
		file_path += '/';
	if (request.path.length() > slashPos)
		file_path += request.path.substr(slashPos + 1);
	if (file_path[file_path.length() - 1] != '/')
		file_path += '/';
	for (unsigned int j = 0; j < ind.size(); j++)
	{
		acss = access((file_path + ind[j]).c_str(), F_OK);
		if (!acss && !access((file_path + ind[j]).c_str(), R_OK))
		{
			status_code = 200;
			return (file_path + ind[j]);
		}
		else if (!acss)
			status_code = 403;
		else if (dotPos != NPOS)
		{
			status_code = 404;
			return ("");
		}
	}
	if (status_code == 403)
		return ("");
	if (it->second.get_autoindex())
	{
		status_code = 1001;
		return (it->second.get_root());
	}
	status_code = 404;
		return ("");
}

std::string	fetch_post_path(HttpRequest &request, std::map<std::string, LocationDir>::iterator it, size_t const &slashPos, int &status_code)
{
	std::string	filepath;

	(void) status_code;
	filepath = it->second.get_root();
	if (filepath[filepath.length() - 1] != '/')
		filepath += '/';
	if (request.path.length() > slashPos)
		filepath += request.path.substr(slashPos + 1);
	if (filepath[filepath.length() - 1] != '/')
		filepath += '/';
	
	return (filepath);
}

std::string get_file_path(HttpRequest &request, Config &config, int &status_code)
{
	std::string file_path;
	unsigned int	i = 0;
	std::map<std::string, LocationDir>	&locations = config.getLocMap(request.port_number);
	std::map<std::string, LocationDir>::iterator	it;
	std::map<std::string, LocationDir>::iterator	locEnd = locations.end();
	std::string	locRoute;
	size_t	slashPos = 1;
	size_t dotPos = NPOS;

	if (request.path.length() > 1)
		slashPos = request.path.find('/', 1);
	dotPos = request.path.find('.');
	if (dotPos != NPOS)
	{
		file_path = file_request_case(slashPos, dotPos, request, config, status_code);
		if (status_code != 200 || !file_path.empty()) return (file_path);
	}
	for (it = locations.begin(); it != locEnd; it++)
	{
		locRoute = it->first;
		if (locRoute == request.path.substr(0, slashPos))
		{
			if (!(it->second.get_redir().empty()))
			{ status_code = 301; return (it->second.get_redir()); }
			break;
		}
	}
	if (it == locEnd && locations.begin() != locEnd)
	{ status_code = 404; return (""); }
	else
	{
		std::vector<std::string> methods = it->second.get_methods_allowed();
		while (i < methods.size() && methods[i] != request.method)
			i++;
		if (i < methods.size())
		{
			if (methods[i] == "GET")
				return (check_index_files(request, it, slashPos, dotPos, status_code));
			else if (methods[i] == "POST")
				return (fetch_post_path(request, it, slashPos, status_code));
		}
		status_code = 405; return ("");
	}
}

std::string	get_directory_listing(std::string & file_path, HttpRequest const &request,
		Config &config) {
	DIR *dir;
	struct dirent *en;
	std::vector<std::string> list;
	std::vector<std::string>::iterator it;
	std::string	output;
	LocationDir	&loc = get_Location_for_Path(request, config);

	dir = opendir(file_path.c_str());
	if (dir) {
		while ((en = readdir(dir)) != NULL) {
			list.push_back(en->d_name);
		}
		closedir(dir);
		output = START_OF_LIST;
		output += "<h1>Directory listing</h1>";
		output += "<ul>";
		for (it = list.begin(); it != list.end(); ++it) {
			output += "<li><a href=\"http://localhost:" + std::to_string(request.port_number)
			+ loc.get_route() + "/" + *it + "\">" + *it + "</a></li>";
		}
		output += "</ul>";
		output += END_OF_LIST;
		return (output);
	} else {
		throw std::ios_base::failure("Error opening the directory!");
	}
}
