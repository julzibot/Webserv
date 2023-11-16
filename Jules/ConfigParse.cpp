#include "ConfigParse.hpp"

// ConfigParse parse_config_file(std::string path)
// {
//     std::ifstream conf_file(path);
//     std::string line;
//     std::string dir;
//     size_t charpos;
//     std::istringstream linestream("");
//     ConfigParse config;

//     while (std::getline(conf_file, line))
//     {
//         charpos = line.find('#');
//         if (charpos != std::string::npos && line.find('"') > charpos)
//         {
//             line = line.substr(0, charpos);
//         }
//         linestream.str(line);
//         linestream >> dir;
//     }
// }

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

int main()
{
	std::string str =	"here \'\" is \"#stuff\' good #\"right ?\n"
						"somestuff #\"yeah\"\n"
						"plus#morestufff\n"
						"##ok\n"
						"it works";
	std::string line;
	std::istringstream ss = std::istringstream(str);
	while (std::getline(ss, line))
		std::cout << "original line: " << line << std::endl << "parsed comments: " << parse_comments(line) << std::endl;
	return (0);
}