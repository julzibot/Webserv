#include "ConfigParse.hpp"

std::string parse_comments(std::string original_line)
{
	std::string line(original_line);
	size_t hashpos;
	size_t quotepos;
	size_t sq;
	size_t sec_quotepos;

	hashpos = line.find('#');
	quotepos = line.find('\"');
	sq = line.find('\'');
	quotepos = sq < quotepos ? sq : quotepos;
	sec_quotepos = 0;
	while (hashpos != std::string::npos)
	{
		if (quotepos != std::string::npos && quotepos < hashpos)
		{
			sec_quotepos = line.substr(quotepos + 1).find(line[quotepos]);
			if (sec_quotepos != std::string::npos)
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
			if (quotepos != std::string::npos) 
				quotepos += sec_quotepos + 1;
		}
		else
		{
			sq = hashpos;
			hashpos = line.substr(hashpos + 1).find('#');
			if (hashpos != std::string::npos)
				hashpos += sq + 1;
		}
	}
	return (line);
}

template <typename T>
void	get_braces_content(std::string dir_key, T &stream, std::unordered_map<std::string, std::string> &directives, std::vector<std::string> &dir_index)
{
	int open_braces = 1;
	std::string line;

	dir_index.push_back(dir_key);
	while (std::getline(stream, line))
	{
		line = parse_comments(line);
		if (line.find('{') != std::string::npos)
			open_braces++;
		else if (line.find('}') != std::string::npos)
			open_braces--;
		if (open_braces)
			directives[dir_key] += line + "\n";
		else
			break;
	}
	if (open_braces)
		throw std::exception();
}

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
	// elsede .
	
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

void parse_config_file(std::string path)
{
	int	i = 0;
	// int	loop = 0;
    std::ifstream conf_file(path);
    std::string line;
    std::string directive = "none";
    std::istringstream ls;
    size_t bracepos;
	std::unordered_map<std::string, std::string> directives;
	std::vector<std::string> dir_index;
    ConfigParse config;

    while (std::getline(conf_file, line) || std::getline(ls, line) || i < dir_index.size())
    {
		if (conf_file.eof() && ls.eof())
		{
			directive = dir_index.at(i);
			ls.clear();
			ls.str(directives[dir_index.at(i)]);
			std::getline(ls, line);
			i++;
		}
		line = parse_comments(line);
		if (line.find("include") != line.npos)
            expandInclude(line, ls);
		bracepos = line.find('{');
		if (bracepos != std::string::npos && !conf_file.eof())
			get_braces_content<std::ifstream>(line.substr(0, bracepos), conf_file, directives, dir_index);
		else if (bracepos != std::string::npos && conf_file.eof())
			get_braces_content<std::istringstream>(line.substr(0, bracepos), ls, directives, dir_index);
		try {
			parseDirective(line, directive, config);
		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
			exit(1);
		}
    }
	conf_file.close();

	// TESTING TYPES UNORDERED MAP:
	int	j = 0;
	for (std::unordered_map<std::string, std::string>::iterator it = config.getBeginTypesMap(); it != config.getEndTypesMap(); ++it) {

		std::cout << "(" << j << ") " << it->first << ": " << it->second << std::endl;
		++j;
	}
	// TESTING PARSING OUTPUT
	// for (i = 0; i < dir_index.size(); i++)
	// 	std::cout << "key: " << dir_index.at(i) << "  value: " << directives[dir_index.at(i)] << std::endl << "----------" << std::endl;
}

int main()
{
	std::string path = "./webserv.conf";
	std::string line;
	// std::ifstream ifs(path);
	std::istringstream ls;
	// std::string s = ("Hello Milan\nhow are you ?");

	parse_config_file(path);
	// ls.str(s);
	// std::cout << ls.str() << std::endl;
	// while (std::getline(ls, line))
	// std::cout << line << std::endl;
	return (0);
}