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
	line = "";
	while (line.empty())
		std::getline(s, line);
}

template <typename T>
void	get_braces_content(std::string dir_key, T &stream, std::unordered_map<std::string, std::string> &directives, std::vector<std::string> &dir_index)
{
	bool add_portnum = 0;
	int open_braces = 1;
	int	i = dir_index.size() - 1;
	std::string line;
	std::string portnum = "";

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
		throw std::exception();
}

void parse_config_file(std::string path)
{
	int	i = 0;
    std::ifstream conf_file(path);
    std::istringstream ls;
    std::string line;
	std::string buffer;
    std::string directive = "main";
    size_t bracepos;
	std::unordered_map<std::string, std::string> directives;
	std::vector<std::string> dir_index;
    ConfigParse config;

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
		// parseDirective(line, directive, config);
    }

	// TESTING PARSING OUTPUT
	for (i = 0; i < dir_index.size(); i++)
		std::cout << "key: " << dir_index.at(i) << "  value: " << directives[dir_index.at(i)] << std::endl << "----------" << std::endl;
}

// std::string ConfigParse::get_file_path(HttpRequest request) const
// {
// 	/**
// 	 * 1. Check the request path.
// 	 * 1.5: Check if the METHOD matches for this path
// 	 * 2. Check the location.
// 	 * 3. Test for file mentioned in index or 
// 	 * 		one obtained by appending the path name.
// 	 * 4. Use the try files directive to find the file.
// 	 * 5. If file is found, return the path.
// 	 * 6. Check if directory listing is ON
// 	*/
// }

int main()
{
	std::string path = "webserv.conf";
	// std::ifstream file(path);

	parse_config_file(path);
	return (0);
}
