/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 18:56:36 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/28 12:17:13 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

std::string	removeSpaces( std::string line ) {

	if (line.empty())
		return (line);
    std::string    spaceless;
    bool        charEnc = false;

    for (unsigned int i = 0; i < line.length(); ++i) {
        if (charEnc == true && isspace(line[i]) &&
			(i != line.length() - 1 && !isspace(line[i + 1])))
            spaceless += " ";
        if (!isspace(line[i]))
        {
            charEnc = true;
            spaceless += line[i];
        }
    }
    return (spaceless);
}

void	auth_except(LocationDir& ld, std::string line) 
{
	std::istringstream	toSegment(line);
	std::string			value;

	while (!toSegment.eof()) {
		
		std::getline(toSegment, value, ' ');
		ld.removeMethod(value);
	}
}

void	assign_autoindex(LocationDir& ld, std::string value) 
{
	if (value == "on")
		ld.setAutoindex(true);
	else if (value == "off")
		ld.setAutoindex(false);
	else
		throw (std::invalid_argument("\"" + value + "\": Unknown parameter for 'autoindex'."));
}

bool	is_valid_IP(std::string const &str)
{
	bool	is_ip = false;
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (!std::isdigit(str[i]) && str[i] != '.')
			return (false);
		else if (str[i] == '.')
			is_ip = true;
	}
	return (is_ip);
}

void	dirParseServer(Config& config, std::string line, std::string directive) 
{
	line = removeSpaces(line);
	if (line.empty())
		return;

	std::istringstream	ls;
	std::string			buff;
	std::string			varName;
	std::vector<int>	ports;
	std::string			hostIP = "";

	ls.str(directive); ls >> buff >> buff;
	if (is_valid_IP(buff) == true)
		hostIP = buff;
	else
		ports.push_back(stoi(buff.substr(0, buff.find('_'))));
	while (ls >> buff)
		ports.push_back(stoi(buff.substr(0, buff.find('_'))));

	ls.clear(); ls.str(line);
	ls >> varName;
	if (varName == "listen")
	{
		std::cout << "PORTS SIZE: " << ports.size() << std::endl;
		// std::vector<int> portnums = config.get_portnums();
		for (unsigned int i = 0; i < ports.size(); i++)
		{
			std::vector<int> portnums = config.get_portnums();
			std::vector<int>::iterator it = std::find(portnums.begin(), portnums.end(), ports[i]);
			if (it == portnums.end())
				config.add_portnum(ports[i]);
			strstrMap &servMain = config.getServMain(hostIP, ports[i], "", false);
			servMain["server_name"] = ""; servMain["root"] = ""; servMain["error_pages"] = "";
		}
	}
	else if (varName == "server_name" || varName == "root" || varName == "error_pages")
	{
		ls >> buff;
		for (unsigned int i = 0; i < ports.size(); i++)
		{
			config.getServMain(hostIP, ports[i], "", false)[varName] = buff;
			if (config.getServMain(hostIP, ports[i], "main", false)[varName].empty())
				config.getServMain(hostIP, ports[i], "main", false)[varName] = buff;
		}
	}
	else
		throw (std::invalid_argument("\"" + varName + "\": Unknown parameter in 'server' directive"));
}

bool	loc_assign(std::string const &keyword, std::string const &value, LocationDir &ld, std::string const &tempRoute)
{
	std::istringstream locdirs("index root methods_except autoindex redirect");
	std::string buff;

	(void)tempRoute;
	while (locdirs >> buff)
		if (buff == keyword) break;
	if (buff.empty())
		throw (std::invalid_argument("\"" + keyword + "\": Unknown parameter in 'location' directive"));
	else if (keyword == "index" && ld.get_index().size() == 0)
		{ ld.setindex(value); return (true); }
	else if (keyword == "root" && ld.get_root().empty())
		{ ld.setRoot(value); return (true); }
	else if (keyword == "methods_except")
		{ auth_except(ld, value); return (true); }
	else if (keyword == "autoindex" && ld.get_autoindex() == false)
		{ assign_autoindex(ld, value); return (true); }
	else if (keyword == "redirect" && ld.get_redir().empty())
		{ ld.setRedir(value); return (true); }
	else
		return (false);
}

void	dirParseLocation(Config &config, std::string line, std::string directive)
{
	std::string	portStr;
	std::string hostIP = "";
	std::string	route;
	std::string	dupStr = "";
	std::vector<int>	ports;
	std::istringstream dirStream(directive);
	dirStream >> route >> route;
	dirStream >> portStr;

	if (directive[directive.length() - 1] == '_')
		dupStr = directive.substr(directive.find('_'));
	
	if (is_valid_IP(portStr) == true)
		hostIP = portStr;
	else
		ports.push_back(stoi(portStr.substr(0, portStr.find('_'))));
	while (dirStream >> portStr)
		ports.push_back(stoi(portStr.substr(0, portStr.find('_'))));

    std::istringstream	linestream(line);
    std::string			keyword;
    std::string			value;
	std::string			tempRoute;


	linestream >> keyword;
	if (line.length() == keyword.length())
		throw (std::invalid_argument("\"" + keyword + "\": Bad line in 'location' directive"));
	value = line.substr(line.find(keyword) + keyword.length() + 1);
	std::cout << "WUT" << std::endl;
	for (unsigned int i = 0; i < ports.size(); i++)
	{
		tempRoute = route;
		for (unsigned int k = 0; k < dupStr.length(); k++)
			tempRoute += " ";
		
		// std::cout << "TEMPROUTE: |" << tempRoute << "|" << std::endl;
		LocationDir	&ld = config.getLocRef(hostIP, ports[i], tempRoute);
		if (ld.get_route().empty())
			ld.setRoute(tempRoute);
		// std::cout << "LOC: |" << tempRoute << "|" << std::endl;
		// std::cout << "VARS: " << keyword << " " << value << std::endl;
			
		if	(loc_assign(keyword, value, ld, tempRoute) == false)
			return;
		strstrMap &ServerMain = config.getServMain(hostIP, ports[i], tempRoute, false);
		if (ServerMain.empty())
		{
			strstrMap &to_assign = config.getServMain(hostIP, ports[i], "", false);
			if (to_assign["root"].empty() || to_assign["error_pages"].empty())
				throw std::invalid_argument("Config file: 'root' or 'error_pages' missing in server settings");
			ServerMain["root"] = to_assign["root"];
			ServerMain["server_name"] = to_assign["server_name"];
			ServerMain["error_pages"] = to_assign["error_pages"];
		}
	}
}

void	dirParseTypes(Config& config, std::string line, std::string directive)
{
	(void)directive;
	if (line.empty())
		return;
	std::istringstream	stream(line);
	std::string	value;
	std::string	extension;
	stream >> value;
	while (stream >> extension)
		config.add_type(extension.substr(0, extension.find(';')), value);

}

void	dirParseCGI(Config& config, std::string line, std::string directive)
{
	(void)directive;
	if (line.empty())
		return;
	std::istringstream	stream(line);
	std::string	value;
	std::string	extension;
	stream >> value;
	while (stream >> extension)
		config.add_cgi(extension.substr(0, extension.find(';')), value);
}

void	dirParseHosts(Config& config, std::string line, std::string directive)
{
	(void)directive;
	if (line.empty())
		return;
	std::istringstream	stream(line);
	std::string	ip;
	std::string	hostname;
	stream >> ip;
	while (stream >> hostname)
		config.add_hosts(hostname.substr(0, hostname.find(';')), ip);
}

void	dirParseMain(Config& config, std::string line, std::string directive)
{
	(void)directive;
	if (line.empty())
		return;
	line = removeSpaces(line);

	std::istringstream	stream(line);
	std::string			var;
	std::string			value;

	stream >> var >> value;
	if (var == "max_bodysize")
		config.set_bodysize(std::atoi(value.c_str()));
	// else
	// 	throw std::invalid_argument("Unknown main directive found");
}

void	parseDirective(std::string line, std::string directive, Config& config) 
{
	std::map<std::string, funcPtr>	dirCase;
	std::string						portnum;
	std::string 					route;
	std::string 					dirKey;

	line = removeSpaces(line);
	if (line.empty())
		return;
	dirCase["location"] = &dirParseLocation;
	dirCase["server"] = &dirParseServer;
	dirCase["types"] = &dirParseTypes;
	dirCase["hosts"] = &dirParseHosts;
	dirCase["main"] = &dirParseMain;
	dirCase["cgi"] = &dirParseCGI;

	directive = removeSpaces(directive);
	std::istringstream(directive) >> dirKey;

	if (dirCase.find(dirKey) != dirCase.end())
		dirCase[dirKey](config, line, directive);
	else if (dirKey != "http")
		throw std::invalid_argument("Unknown main directive found");
}
