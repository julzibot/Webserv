/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 18:56:36 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/29 19:36:32 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DirectiveParsing.h"

std::string    removeSpaces( std::string line ) {

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

void	auth_except(LocationDir& ld, std::string line) {

	std::istringstream	toSegment(line);
	std::string			value;

	while (!toSegment.eof()) {
		
		std::getline(toSegment, value, ' ');
		ld.removeMethod(value);
	}
}

void	assign_autoindex(LocationDir& ld, std::string value) {

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
	int	port = std::atoi(portStr.c_str());

    std::istringstream	linestream(line);
    std::string			keyword;
    std::string			value;
    LocationDir			&ld = config.getLocRef(port, route);

    linestream >> keyword;
    if (keyword == "index") {
        value = line.substr(line.find(keyword) + 6);
        ld.setindex(value);
    }
    else if (keyword == "root") {
    	value = line.substr(line.find(keyword) + 5);
    	ld.setRoot(value);
    }
	else if (keyword == "methods_except") {
		value = line.substr(line.find(keyword) + 15);
		auth_except(ld, value);
	}
	else if (keyword == "autoindex") {
		value = line.substr(line.find(keyword) + 10);
		assign_autoindex(ld, value);
	}
	else
		throw (std::invalid_argument("Unkown parameter 'location'."));

	// if (!ld.get_index().empty())
	// 	std::cout << "Index: " << ld.get_index().at(0) << std::endl;
	// std::cout << "Root: " << ld.get_root() << std::endl;

    // std::map<int, std::map<std::string, LocationDir> > server = config.getServ();
    // std::vector<std::string> ind = ld.get_index();
    // for (int i = 0; i < ind.size(); i++)
    //     std::cout << ind.at(i) << std::endl;
}

void	dirParseEvents(Config& config, std::string line, std::string directive) {

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

void	dirParseTypes(Config& config, std::string line, std::string directive) {

	(void)directive;
	if (line.empty())
		return;
	line = removeSpaces(line);

	std::istringstream	stream(line);
	while (!stream.eof()) {

		std::string	typeLine;
		std::getline(stream, typeLine);
		std::vector<std::string>	vectorTypeLine;
		std::string					segmented;
		std::istringstream			vectorStream(typeLine);

		while (std::getline(vectorStream, segmented, ' '))
			vectorTypeLine.push_back(segmented);

		std::vector<std::string>::iterator it;
		std::string		value = *vectorTypeLine.begin();

		for (it = vectorTypeLine.begin(); it != vectorTypeLine.end(); ++it) {
	
			if (it != vectorTypeLine.begin())
				config.add_type(*it, value);
		}
	}
}

void	dirParseMain(Config& config, std::string line, std::string directive) {

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

bool	validErrorHtmlFile( std::string filename ) {
	
	std::fstream	verifyFile(filename);

	// if (!verifyFile.good())
	// 	throw (std::invalid_argument("Problem with 'error_page' file."));
	if (filename.find(".html") == NPOS)
		throw (std::invalid_argument("Problem with 'error_page' file extension."));
	return (true);
}

bool	isValidErrCode( const std::string& errCode ) {
	
	if (errCode.length() > 3 || errCode.empty())
		return (false);
	for (int i = 0; i < errCode.length(); ++i) {
		if (errCode[i] < '0' || errCode[i] > '9')
			return (false);
	}
	return (true);
}

void	dirParseServer(Config& config, std::string line, std::string directive) {

	line = removeSpaces(line);
	if (line.empty())
		return;

	std::string	portStr;
	std::istringstream(directive) >> portStr >> portStr;
	int	port = std::atoi(portStr.c_str());

	std::istringstream	stream(line);
	std::string			varName;

	stream >> varName;
	if (varName == "error_page") {
		
		std::vector<std::string>	arr;
		std::string					tmp;

		std::getline(stream, tmp, ' ');
		while (std::getline(stream, tmp, ' '))
			arr.push_back(tmp);

		std::map<int, std::string>	codesMap;
		std::string					htmlFilename;

		if (validErrorHtmlFile(arr[arr.size() - 1]))
			htmlFilename = arr[arr.size() - 1];

		for (int i = 0; i < arr.size() - 1; ++i) {

			if (isValidErrCode(arr[i]))
				codesMap[std::atoi(arr[i].c_str())] = htmlFilename;
			else
				throw (std::invalid_argument("Bad error code value in 'server'."));
		}

		config.getErrorMap()[port] = codesMap;
	}
	else if (varName != "listen")
		throw (std::invalid_argument("Unknown 'server' parameter."));
}

void	initDirMap(std::map<std::string, funcPtr>& dirCase) {

	dirCase["location"] = &dirParseLocation;
	dirCase["server"] = &dirParseServer;
	dirCase["events"] = &dirParseEvents;
	dirCase["types"] = &dirParseTypes;
	dirCase["main"] = &dirParseMain;
}

void	parseDirective(std::string line, std::string directive,
	Config& config) 
{
	std::map<std::string, funcPtr>	dirCase;
	std::string						portnum;
	std::string 					route;
	std::string 					dirKey;

	initDirMap(dirCase);
	directive = removeSpaces(directive);
	std::istringstream(directive) >> dirKey;

	if (dirCase.find(dirKey) != dirCase.end())
		dirCase[dirKey](config, line, directive);
	else if (dirKey != "http")
		throw ("Unknown directive found.");
}

// int main()
// {
//     // dirParseLocation(1, "/","index file1 file2 file3");
// 	// std::string line;
// 	// 	std::cout << line << std::endl;
// 	// 	line = removeSpaces(line);


//     return (0);
// }