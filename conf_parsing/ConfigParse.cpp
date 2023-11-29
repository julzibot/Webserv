/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:27:12 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/28 15:41:19 by mstojilj         ###   ########.fr       */
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
	filename = filename.substr(0, filename.find(';'));
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
			portnum = " " + portnum.substr(0, portnum.find(';'));
			dir_index.push_back(dir_key + portnum);
			add_portnum = 0;
		}
		if (line.find('{') != NPOS)
		{
			open_braces++;
			dir_key = line.substr(0, line.find('{'));
			if (dir_key.find("server") != NPOS)
				add_portnum = 1;
			if (!add_portnum)
				dir_index.push_back(dir_key + portnum);
		}
		else if (line.find('}') != NPOS && --open_braces > 0)
			dir_key = dir_index.at(i + open_braces);
		else if (open_braces)
			directives[dir_key + portnum] += line + "\n";
	}
	if (open_braces)
		throw std::invalid_argument("Braces not closed.");
}

Config	parse_config_file(std::string path)
{
	int							i = 0;
    size_t						bracepos;
    Config						config;
    std::string					line;
	std::string					buffer;
    std::string					directive = "main";
	strstrMap					directives;
    std::ifstream				conf_file(path);
    std::istringstream			ls;
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
		bracepos = line.find('{');
		if (bracepos != NPOS)
			get_braces_content<std::istringstream>(line.substr(0, bracepos), ls, directives, dir_index);
		parseDirective(line, directive, config);
    }

	// TESTING PARSING OUTPUT
	// for (i = 0; i < dir_index.size(); i++)
	// 	std::cout << "\e[31mkey: \e[0m" << dir_index.at(i) << "  \e[33mvalue: \e[0m"
	// 		<< directives[dir_index.at(i)] << std::endl << "\e[34m----------\e[0m" << std::endl;
	return (config);
}

std::string get_file_path(HttpRequest request, Config &config)
{
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
	std::string file_path;
	unsigned int	i = 0;

	std::map<std::string, LocationDir>	&locations = config.getLocMap(request.port_number);
	std::map<std::string, LocationDir>::iterator	it = locations.begin();
	std::map<std::string, LocationDir>::iterator	locEnd = locations.end();
	std::vector<std::string>	methods;
	methods.push_back("GET");
	// std::vector<int, std::string>	servnums = config.ge
	// std::cout << "1 " << it->first << " | " << request.path << std::endl;
	// std::cout << request.path << " " << request.method << " " << request.http_version << std::endl;
	while (it != locEnd && it->second.get_route() != request.path)
	{
		std::cout << "| " << it->second.get_route() << " | " << std::endl;
		it++;
	}
	// std::cout << "here: " << it->second.get_route() << std::endl; 
	if (it == locEnd && locations.begin() != locEnd)
		std::cout << "problem encountered" << std::endl;//error handling
	else
	{
		std::vector<std::string>::iterator method_iterator;
		std::cout << 1 << std::endl;
		while (i < methods.size() && methods[i] != request.method)
			i++;
		std::cout << 2 << std::endl;
		if (i < methods.size())
		{
			// Test the presence of the file for different
			// indexes and return the one that exists;
			std::vector<std::string> ind = it->second.get_index();
			std::cout << 3 << std::endl;
			for (int j = 0; j < ind.size(); j++)
			{
				file_path = removeSpaces(it->second.get_root()) + request.path + ind[j];
				// std::cout << file_path << std::endl;
				if (!access(file_path.c_str(), R_OK))
					return (file_path);
			}
			std::cout << "no valid file encountered" << std::endl;
			return ("error_3");
			// directory listing;
		}
		else
			std::cout << "method not allowed" << std::endl;
		return ("error_2");
	}
	return ("error_1");
	// If code reaches this section, the route and method do not exist.
	// TODO: Throw an error for route/path not existing.
}

// int main()
// {
// 	std::string path = "webserv.conf";
// 	// std::ifstream file(path);

// 	Config c = parse_config_file(path);
// 	return (0);
// }
