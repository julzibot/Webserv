/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/23 18:56:36 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/24 15:49:11 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <map>
#include <sstream>
#include "DirectiveParsing.h"
#include "ConfigParse.hpp"
#include "LocationDir.hpp"

void dirParseLocation(int port, std::string route, std::string line, ConfigParse &config)
{
    LocationDir &ld = config.getLocRef(port, route);
    std::istringstream linestream(line);
    std::string keyword;
    std::string value;

    linestream >> keyword;
    if (keyword == "index") {
        value = line.substr(line.find(keyword) + 6);
        std::cout << "|" << value << "|" << std::endl;
        ld.setindex(value);
    }
    if (keyword == "root") {
        value = line.substr(line.find(keyword) + 5);
        ld.setRoot(value);
    }

    // std::unordered_map<int, std::unordered_map<std::string, LocationDir> > server = config.getServ();
    // std::vector<std::string> ind = ld.get_index();
    // for (int i = 0; i < ind.size(); i++)
    //     std::cout << ind.at(i) << std::endl;
}

std::string    removeSpaces( std::string line ) {

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

void	dirParseEvents(ConfigParse& config, std::string line) {

	if (line.empty())
		return;
	line = removeSpaces(line);
	if (line.find(';') != line.npos)
		line.erase(line.find(';'));


	std::istringstream	stream(line);
	std::string	var;
	std::string	value;

	stream >> var >> value;
	if (var == "worker_connections")
		config.set_workco(std::atoi(value.c_str()));
	else
		throw (std::invalid_argument("Bad 'events' parameter found."));
}

// void	dirParseHttp(ConfigParse& config, std::string line) {

// 	line = removeSpaces(line);
// 	line.erase(line.find(';'));

// 	std::istringstream	stream(line);
// 	std::string	var;
// 	std::string	value;

// 	stream >> var >> value;
// 	// if (var == "worker_connections")
// 	// 	config.setWorkerCnts(std::atoi(value.c_str()));
// 	// else
// 	// 	throw (std::invalid_argument("Bad 'events' parameter found."));
// }

void	dirParseTypes(ConfigParse& config, std::string line) {

	if (line.empty())
		return;
	line = removeSpaces(line);
	if (line.find(';') != line.npos)
		line.erase(line.find(';'));

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

void	dirParseMain(ConfigParse& config, std::string line) {

	if (line.empty())
		return;
	line = removeSpaces(line);
	if (line.find(';') != line.npos)
		line.erase(line.find(';'));

	std::istringstream	stream(line);
	std::string	var;
	std::string	value;

	stream >> var >> value;
	if (var == "worker_processes")
		config.set_workproc(std::atoi(value.c_str()));
	// else
	// 	throw (std::invalid_argument("Bad parameter found."));
}

void	initDirMap(std::map<std::string, funcPtr>& dirCase) {

	dirCase["events"] = &dirParseEvents;
	// dirCase.insert(std::pair<std::string, funcPtr>("http", &dirParseHttp));
	// dirCase.insert(std::pair<std::string, funcPtr>("server", &dirParseServer));
	dirCase["types"] = &dirParseTypes;
	// dirCase.insert(std::pair<std::string, funcPtr>("location", &dirParseLocation));
	dirCase["none"] = &dirParseMain;
}

void	parseDirective(std::string& line, std::string& directive,
			ConfigParse& config) {

	std::map<std::string, funcPtr>	dirCase;

	initDirMap(dirCase);
	directive = removeSpaces(directive);
	if (dirCase.find(directive) != dirCase.end())
		dirCase[directive](config, line);
	// else if (directive == "location")
	// 	machin
	// else
	// 	throw ("Unknown directive found.");
}

// int main()
// {
//     dirParseLocation(1, "/","index file1 file2 file3");

//     return (0);
// }