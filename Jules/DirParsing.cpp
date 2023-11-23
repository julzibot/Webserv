#include "ConfigParse.hpp"

void dirParseLocation(int port, std::string route, std::string line, ConfigParse &config)
{
    LocationDir &ld = config.getLocRef(port, route);
    std::istringstream linestream(line);
    std::string keyword;
    std::string value;

    linestream >> keyword;
    if (keyword == "index")
    {
        value = line.substr(line.find(keyword) + 1);
        std::cout << value << std::endl;
        ld.setindex(value);
    }

    // std::unordered_map<int, std::unordered_map<std::string, LocationDir> > server = config.getServ();
    // std::vector<std::string> ind = ld.get_index();
    // for (int i = 0; i < ind.size(); i++)
    //     std::cout << ind.at(i) << std::endl;
}

// int main()
// {
//     dirParseLocation(1, "/","index file1 file2 file3");

//     return (0);
// }