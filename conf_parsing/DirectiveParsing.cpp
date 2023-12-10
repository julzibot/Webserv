/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 18:56:36 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/10 21:13:12 by julzibot         ###   ########.fr       */
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
		throw (std::invalid_argument("Unknown parameter in 'autoindex'."));
}

void	dirParseLocation(Config &config, std::string line, std::string directive)
{
	std::string	portStr;
	std::string	route;
	std::istringstream(directive) >> route >> route >> portStr;
	int	port = stoi(portStr);

    std::istringstream	linestream(line);
    std::string			keyword;
    std::string			value;
    LocationDir			&ld = config.getLocRef(port, route);

	ld.setRoute(route);
    linestream >> keyword;
	// std::cout << "| " << keyword << " |" << std::endl;
    if (keyword == "index") {
        value = removeSpaces(line.substr(line.find(keyword) + 6));
        ld.setindex(value);
    }
    else if (keyword == "root") {
    	value = removeSpaces(line.substr(line.find(keyword) + 5));
    	ld.setRoot(value);
    }
	else if (keyword == "methods_except") {
		value = removeSpaces(line.substr(line.find(keyword) + 15));
		auth_except(ld, value);
	}
	else if (keyword == "autoindex") {
		value = removeSpaces(line.substr(line.find(keyword) + 10));
		assign_autoindex(ld, value);
	}
	else if (keyword == "redirect") {
		value = removeSpaces(line.substr(line.find(keyword) + 9));
		ld.setRedir(value);
	}
	else
		throw (std::invalid_argument("Unknown parameter in 'location' directive"));
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
		throw (std::invalid_argument("Unknown parameter in 'events'."));
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

bool	validErrorHtmlFile( std::string filename ) 
{	
	std::fstream	verifyFile(filename);

	// if (!verifyFile.good())
	// 	throw (std::invalid_argument("Problem with 'error_page' file."));
	if (filename.find(".html") == NPOS)
		throw (std::invalid_argument("Problem with 'error_page' file extension."));
	return (true);
}

bool	isValidErrCode( const std::string& errCode ) 
{	
	if (errCode.length() > 3 || errCode.empty())
		return (false);
	for (int i = 0; i < errCode.length(); ++i) {
		if (errCode[i] < '0' || errCode[i] > '9')
			return (false);
	}
	return (true);
}

void	dirParseServer(Config& config, std::string line, std::string directive) 
{
	line = removeSpaces(line);
	if (line.empty())
		return;

	std::string	portStr;
	std::istringstream(directive) >> portStr >> portStr;
	int	port = stoi(portStr);

	std::istringstream	stream(line);
	std::string			varName;

	stream >> varName;
	if (varName == "error_pages")
	{
		std::string &err = config.getErrorPath(port);
		stream >> err;
	}
	else if (varName == "listen")
		config.add_portnum(port);
	else
		throw (std::invalid_argument("Unknown 'server' parameter."));
}

void	initDirMap(std::map<std::string, funcPtr>& dirCase) {

	dirCase["location"] = &dirParseLocation;
	dirCase["server"] = &dirParseServer;
	dirCase["events"] = &dirParseEvents;
	dirCase["types"] = &dirParseTypes;
	dirCase["main"] = &dirParseMain;
}

void	parseDirective(std::string line, std::string directive, Config& config) 
{
	std::map<std::string, funcPtr>	dirCase;
	std::string						portnum;
	std::string 					route;
	std::string 					dirKey;

	if (removeSpaces(line).empty())
		return;
	initDirMap(dirCase);
	directive = removeSpaces(directive);
	std::istringstream(directive) >> dirKey;

	if (dirCase.find(dirKey) != dirCase.end())
		dirCase[dirKey](config, line, directive);
	else if (dirKey != "http")
		throw ("Unknown directive found.");
}
