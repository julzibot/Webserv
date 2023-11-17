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
void	get_braces_content(std::string dir_key, T &stream, std::map<std::string, std::string> &directives)
{
	int open_braces = 1;
	std::string line;

	// std::cout << "BRACE" << std::endl;
	while (std::getline(stream, line) && open_braces)
	{
		line = parse_comments(line);
		if (line.find('{') != std::string::npos)
			open_braces++;
		else if (line.find('}') != std::string::npos)
			open_braces--;
		if (open_braces)
			directives[dir_key] += line + "\n";
	}
	if (open_braces)
		throw std::exception();
}

void parse_config_file(std::string path)
{
	bool dir = 0;
	int	loop = 0;
    std::ifstream conf_file(path);
    std::string line;
    std::string directive = "none";
    std::istringstream ls;
    size_t bracepos;
	std::map<std::string, std::string> directives;
	std::map<std::string, std::string>::iterator dir_it = directives.begin();
    ConfigParse config;

    while (std::getline(conf_file, line) || std::getline(ls, line) || (dir && dir_it != directives.end()))
    {
		loop++;
		if (!dir && directives.begin() != directives.end())
		{
			dir_it = directives.begin();
			dir = 1;
		}
		if (conf_file.eof() && ls.eof())
		{
			directive = dir_it->first;
			ls.str(dir_it->second);
			// std::cout << ls.str() << "loop: " << loop << std::endl;
			std::getline(ls, line);
			// std::cout << line << "loop: " << loop << std::endl;
			dir_it++;
		}
		line = parse_comments(line);
		//Milan's function
		bracepos = line.find('{');
		if (bracepos != std::string::npos && !conf_file.eof())
			get_braces_content<std::ifstream>(line.substr(0, bracepos), conf_file, directives);
		else if (bracepos != std::string::npos && conf_file.eof())
			get_braces_content(line.substr(0, bracepos), ls, directives);
		// parse_config_line(line, directive, config);
    }
	// std::cout << "end" << std::endl;
	// for (dir_it = directives.begin(); dir_it != directives.end(); dir_it++)
	// 	std::cout << "key: " << dir_it->first << "  value: " << dir_it->second << std::endl;
}

int main()
{
	std::string path = "./webserv.conf";
	std::string line;
	std::istringstream ls("Hello Milan\n"
						"how are you ?");

	parse_config_file(path);
	// while (std::getline(ls, line))
	// 	std::cout << line << std::endl;
	return (0);
}