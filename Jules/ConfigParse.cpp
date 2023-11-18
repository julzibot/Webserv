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

void parse_config_file(std::string path)
{
	int	i = 0;
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
		//Milan's function
		bracepos = line.find('{');
		if (bracepos != std::string::npos && !conf_file.eof())
			get_braces_content<std::ifstream>(line.substr(0, bracepos), conf_file, directives, dir_index);
		else if (bracepos != std::string::npos && conf_file.eof())
			get_braces_content<std::istringstream>(line.substr(0, bracepos), ls, directives, dir_index);
		// parse_config_line(line, directive, config);
    }
	conf_file.close();

	// TESTING PARSING OUTPUT
	for (i = 0; i < dir_index.size(); i++)
		std::cout << "key: " << dir_index.at(i) << "  value: " << directives[dir_index.at(i)] << std::endl << "----------" << std::endl;
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