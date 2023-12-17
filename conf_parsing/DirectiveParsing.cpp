/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 18:56:36 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/17 01:44:04 by julzibot         ###   ########.fr       */
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

void	dirParseServer(Config& config, std::string line, std::string directive) 
{
	line = removeSpaces(line);
	if (line.empty())
		return;

	std::istringstream	ls;
	std::string			buff;
	std::string			varName;
	std::vector<int>	ports;

	ls.str(directive); ls >> buff;
	while (ls >> buff)
		ports.push_back(stoi(buff));

	ls.clear(); ls.str(line);
	ls >> varName;
	if (varName == "listen")
	{
		for (unsigned int i = 0; i < ports.size(); i++)
		{
			std::vector<int> fuck = config.get_portnums();
			std::vector<int>::iterator it = std::find(fuck.begin(), fuck.end(), ports[i]);
			if (it == fuck.end())
				config.add_portnum(ports[i]);
		}
	}
	else if (varName == "server_name" || varName == "root" || varName == "error_pages")
	{
		ls >> buff;
		for (unsigned int i = 0; i < ports.size(); i++)
		{
			config.getServMain(ports[i], "", false)[varName] = buff;
			if (config.getServMain(ports[i], "main", false)[varName].empty())
				config.getServMain(ports[i], "main", false)[varName] = buff;
		}
	}
	else
		throw (std::invalid_argument("\"" + varName + "\": Unknown parameter in 'server' directive"));
}

void	dirParseLocation(Config &config, std::string line, std::string directive)
{
	std::string	portStr;
	std::string	route;
	std::vector<int>	ports;
	std::istringstream dirStream(directive);
	dirStream >> route >> route;
	while (dirStream >> portStr)
		ports.push_back(stoi(portStr));

    std::istringstream	linestream(line);
    std::string			keyword;
    std::string			value;


	linestream >> keyword;
	if (line.length() == keyword.length())
		throw (std::invalid_argument("\"" + keyword + "\": Bad line in 'location' directive"));
	value = line.substr(line.find(keyword) + keyword.length() + 1);
	for (unsigned int i = 0; i < ports.size(); i++)
	{
		strstrMap &ServerMain = config.getServMain(ports[i], route, false);
		if (ServerMain.empty())
		{
			strstrMap to_assign = config.getServMain(ports[i], "", false);
			ServerMain["root"] = to_assign["root"];
			ServerMain["server_name"] = to_assign["server_name"];
			ServerMain["error_pages"] = to_assign["error_pages"];
		}
		LocationDir	&ld = config.getLocRef(ports[i], route);
		ld.setRoute(route);
		
		if (keyword == "index")
			ld.setindex(value);
		else if (keyword == "root")
			ld.setRoot(value);
		else if (keyword == "methods_except")
			auth_except(ld, value);
		else if (keyword == "autoindex")
			assign_autoindex(ld, value);
		else if (keyword == "redirect")
			ld.setRedir(value);
		else
			throw (std::invalid_argument("\"" + keyword + "\": Unknown parameter in 'location' directive"));
	}
}

void	dirParseEvents(Config& config, std::string line, std::string directive)
{
	(void)directive;
	if (line.empty())
		return;
	line = removeSpaces(line);

	std::istringstream	stream(line);
	std::string			var;
	std::string			value;

	stream >> var >> value;
	if (var == "worker_connections")
		config.set_workco(std::atoi(value.c_str()));
	else
		throw (std::invalid_argument("\"" + var + "\": Unknown parameter in 'events' directive."));
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
	if (var == "worker_processes")
		config.set_workproc(std::atoi(value.c_str()));
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
	dirCase["events"] = &dirParseEvents;
	dirCase["types"] = &dirParseTypes;
	dirCase["main"] = &dirParseMain;

	directive = removeSpaces(directive);
	std::istringstream(directive) >> dirKey;

	if (dirCase.find(dirKey) != dirCase.end())
		dirCase[dirKey](config, line, directive);
	else if (dirKey != "http")
		throw ("Unknown main directive found.");
}
