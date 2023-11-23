/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   directiveParsing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/22 19:39:09 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/22 19:40:08 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Needed variables :
// int				worker_processes;
// int				worker_connections;
// ConfigServer	server;
// std::unordered_map<std::string, std::string>	types;

// Needed functions:
// std::unordered_map<std::string, std::string>::iterator	getBeginTypesMap( void ) { return (this->types.begin()); };
// std::unordered_map<std::string, std::string>::iterator	getEndTypesMap( void ) { return (this->types.end()); };
// void	addToTypes(std::string key, std::string value ) { this->types[key] = value; };

template <typename T>
void    expandInclude(std::string &line, T &s) 
{
    std::istringstream    toParse(line);
	std::string		str = "";
    std::string		command;
	std::string		filename;
    std::string		fileLine;
    std::string		fileContent;

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
	std::getline(s, line);
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
		config.setWorkerCnts(std::atoi(value.c_str()));
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
				config.addToTypes(*it, value);
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
		config.setWorkerPrcs(std::atoi(value.c_str()));
	// else
	// 	throw (std::invalid_argument("Bad parameter found."));
}

typedef	void	(*funcPtr)(ConfigParse& config, std::string line);

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
