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

int main()
{
	std::string str = "somestuff #\"yeah\"\n"
						"plus#morestufff\n"
						"##ok\n"
						"it works\n"
						"here \"\" is \"#stuff\" good #\"right ?\"";

	std::string line;
	std::istringstream ss = std::istringstream(str);
	size_t hashpos;
	size_t quotepos;
	size_t sec_quotepos = 0;

	while (std::getline(ss, line))
	{
		std::cout << line;
		hashpos = line.find('#');
		quotepos = line.find('"');

		while (hashpos != std::string::npos)
		{
			if (quotepos != std::string::npos)
			{
				sec_quotepos = line.substr(quotepos + 1).find('"');
				if (sec_quotepos == std::string::npos)
					throw std::exception();
			}
			std::cout << quotepos << " " << sec_quotepos << " " << hashpos << std::endl;
			if (quotepos > hashpos)
			{
				line = line.substr(0, hashpos);
				break;
			}
			else if (quotepos < hashpos && sec_quotepos < hashpos)
				quotepos = line.substr(sec_quotepos + 1).find('"');
			else
				hashpos = line.substr(hashpos + 1).find('#');
		}
		
		std::cout << line << std::endl;
	}
	return (0);
}