#include "Config.hpp"
#include "DirectiveParsing.h"

typedef std::map<std::string, std::string> strstrMap;

std::string parse_comments(std::string original_line)
{
	std::string line(original_line);
	size_t 		hashpos;
	size_t 		quotepos;
	size_t 		sq;
	size_t 		sec_quotepos;

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
    std::istringstream	toParse(line);
	std::string			str = "";
    std::string			command;
	std::string			filename;
    std::string			fileLine;
    std::string			fileContent;

	toParse >> command >> filename;
	if (command != "include")
		return;
    std::ifstream    fs("./conf_parsing/" + filename);
    if (fs.fail())
        throw (std::invalid_argument("Config file: Bad include filename."));
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

size_t	isBrace(char brace, std::string line)
{
	size_t	braceRes = line.find(brace);

	if (braceRes != NPOS) 
	{
		unsigned int i;
		size_t	len = line.length();
		for (i = 0; i < len; i++)
		{
			if (brace == '{' && ((i > braceRes && !isspace(line[i])) \
					|| removeSpaces(line.substr(0, braceRes)).empty())) break;
			else if (brace == '}' && i != braceRes && !isspace(line[i])) break;
		}
		if (i < len)
			throw (std::invalid_argument("Config file: incorrect brace position."));
	}
	return (braceRes);
}

template <typename T>
void	get_braces_content(std::string dir_key, T &stream, std::map<std::string, std::string> &directives, std::vector<std::string> &dir_index, std::vector<std::string> &stringPorts, unsigned int	&add_underscore)
{
	bool		add_portnum = false;
	bool		dupPorts = false;
	int			open_braces = 1;
	bool		can_write = true;
	std::string	line;
	std::string testline;
	std::istringstream	portline;
	std::string portbuff;
	std::string	portnum = "";
	std::vector<std::string>::iterator it;

	dir_index.push_back(dir_key);
	while (open_braces && std::getline(stream, line))
	{
		line = parse_comments(line);
		testline = line;
		testline.erase(std::remove(testline.begin(), testline.end(), ' '), testline.end());
		if (add_portnum && line.find("listen") != NPOS)
		{
			portnum = "";
			portline.str(line); portline >> portbuff;
			while (portline >> portbuff)
			{
				portnum += " " + portbuff;
				if (std::find(stringPorts.begin(), stringPorts.end(), portbuff) == stringPorts.end())
					stringPorts.push_back(portbuff);
				else if (dupPorts == false)
					{ add_underscore += 1; dupPorts = true; }
			}
			for (unsigned int j = 0; j < add_underscore; j++)
				portnum += "_";
			dir_index.push_back(dir_key + portnum);
			portline.clear();
			add_portnum = 0;
		}
		else if (add_portnum && !testline.empty() && line.find("listen") == NPOS)
			throw std::invalid_argument("Config file: directive following server is not 'listen'");
		else if (!add_portnum && !testline.empty() && line.find("listen") != NPOS)
			throw std::invalid_argument("Config file: 'listen' directive at wrong line");
		if (testline.empty())
			;
		else if (isBrace('{', line) != NPOS)
		{
			open_braces++;
			can_write = true;
			dir_key = line.substr(0, line.find('{'));
			if (dir_key.find("server") != NPOS)
				add_portnum = 1;
			if (!add_portnum)
				dir_index.push_back(dir_key + portnum);
		}
		else if (isBrace('}', line) != NPOS)
		{
			open_braces--; can_write = false;
		}
		else if (open_braces && !can_write && !removeSpaces(line).empty())
			throw std::invalid_argument("Config file: \"" + removeSpaces(line) + "\": wrong line place");
		else if (open_braces)
			directives[dir_key + portnum] += line + "\n";
	}
	if (open_braces)
		throw std::invalid_argument("Config file: Unclosed braces found.");
}

Config	parse_config_file( std::string path )
{

    Config						config;
	unsigned int				i = 0;
	unsigned int				add_underscore = 0;
    size_t						bracepos;
    std::istringstream			ls;
	strstrMap					directives;
    std::string					line;
	std::string					buffer;
    std::string					directive = "main";
	std::vector<std::string>	dir_index;
	std::vector<std::string>	stringPorts;
    std::ifstream				conf_file(path);
	
	if (!conf_file.good())
		throw std::invalid_argument("Invalid config file name");
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
		if (line.find("include ") != NPOS)
            expandInclude(line, ls);
		bracepos = isBrace('{', line);
		if (bracepos != NPOS)
			get_braces_content<std::istringstream>(line.substr(0, bracepos), ls, directives, dir_index, stringPorts, add_underscore);
		parseDirective(line, directive, config);
	}
	if (config.get_hostMap().empty() || config.getServ().empty())
		throw	std::invalid_argument("Not enough information in config file");
	return (config);
}

LocationDir& get_Location_for_Path(HttpRequest &request, Config &config)
{
	std::string	reqHost = request.hostIP;
	std::map<std::string, LocationDir>	*locations = config.getLocMap(reqHost, request.port_number);
	std::map<std::string, LocationDir>::iterator	it = locations->begin();
	std::map<std::string, LocationDir>::iterator	locEnd = locations->end();
	std::string	locRoute;

	while (it != locEnd)
	{
		locRoute = it->first;
		if (locRoute == request.path)
			return (it->second);
		++it;
	}
	return (it->second);
}

std::string	file_request_case(size_t &dotPos, HttpRequest &request, Config &config, int &status_code)
{
	std::string	reqHost = request.hostIP;
	std::string file_path;
	int	acss;

	std::string p = request.path;
	file_path = config.getServMain(reqHost, request.port_number, p, true)["root"] + p;
	acss = access(file_path.c_str(), F_OK);
	if (!acss && !access(file_path.c_str(), R_OK))
		return (file_path);
	else if (!acss)
	{ status_code = 403; return ("");}
	else
	{
		while (request.path[--dotPos] != '/') ;
		request.prio_file = request.path.substr(dotPos + 1);
		request.path = request.path.substr(0, dotPos);
		return ("");
	}
}

std::string	check_index_files(HttpRequest &request, std::map<std::string, LocationDir>::iterator it, size_t const &slashPos, size_t const &dotPos, int &status_code)
{
	std::vector<std::string> ind;
	std::string	file_path;
	std::string test_path;
	int acss;

	ind = it->second.get_index();
	if (!request.prio_file.empty())
		ind.insert(ind.begin(), request.prio_file);
	file_path = it->second.get_root();
	test_path = file_path;
	if (file_path[file_path.length() - 1] == '/')
		test_path = file_path.substr(0, file_path.length() - 1);
	if (access(test_path.c_str(), F_OK) == -1)
	{
		status_code = 404;
		return ("");
	}
	if (file_path[file_path.length() - 1] != '/')
		file_path += '/';
	if (request.path.length() > slashPos)
		file_path += request.path.substr(slashPos + 1);
	if (file_path[file_path.length() - 1] != '/')
		file_path += '/';
	for (unsigned int j = 0; j < ind.size(); j++)
	{
		acss = access((file_path + ind[j]).c_str(), F_OK);
		if (!acss && !access((file_path + ind[j]).c_str(), R_OK))
		{
			status_code = 200;
			return (file_path + ind[j]);
		}
		else if (!acss)
			status_code = 403;
		else if (dotPos != NPOS)
		{
			status_code = 404;
			return ("");
		}
	}
	if (status_code == 403)
		return ("");
	if (it->second.get_autoindex())
	{
		status_code = 1001;
		return (it->second.get_root());
	}
	status_code = 404;
	return ("");
}

std::vector<bool>	get_match_vect(std::string const &tempHost, HttpRequest &request, std::map<std::string, LocationDir>	&locations, Config &config)
{
	std::vector<bool>			matching_host;
	std::string					servername;
	std::vector<std::string>	hostnames;
	unsigned int				j;
	std::map<std::string, LocationDir>::iterator	it;

	std::string reqHost = request.headers["Host"];
	reqHost = reqHost.substr(0,reqHost.find(':'));
	reqHost.erase(std::remove(reqHost.begin(), reqHost.end(), '\r'), reqHost.end());

	std::map<std::string, LocationDir>::iterator	locEnd = locations.end();
	strstrMap hostMap = config.get_hostMap();
	strstrMap::iterator h;
	strstrMap::iterator	hEnd = hostMap.end();

	for (h = hostMap.begin(); h != hEnd; h++)
	{
		if (reqHost == h->first || reqHost == h->second)
			hostnames.push_back(h->first);
	}
	for (it = locations.begin(); it != locEnd; it++)
	{
		servername = config.getServMain(tempHost, request.port_number, it->first, true)["server_name"];
		for (j = 0; j < hostnames.size(); j++)
		{
			if (servername == hostnames[j] || servername.empty())
			{
				matching_host.push_back(true);
				if (matching_host[0] != true || matching_host.size() == 1)
					matching_host.insert(matching_host.begin(), true);
				break;
			}
		}
		if (j == hostnames.size())
			matching_host.push_back(false);
	}
	return (matching_host);
}

void	request_ip_check(std::string &reqHost, Config &config, int &status_code)
{
	strstrMap	hostMap = config.get_hostMap();
	strstrMap::iterator it;
	strstrMap::iterator	mapEnd = hostMap.end();

	for (it = hostMap.begin(); it != mapEnd; it++)
	{
		if (reqHost == it->second)
			break;
	}
	if (it == mapEnd)
	{
		for (it = hostMap.begin(); it != mapEnd; it++)
		{
			if (reqHost == it->first)
			{
				reqHost = it->second;
				break;
			}
		}
		if (it == mapEnd)
			status_code = 403;
	}
}

std::map<std::string, LocationDir>	*getNullLocation(Config &config, HttpRequest &request, int &status_code, std::vector<bool>	&matching_host)
{
	std::map<std::string, LocationDir>	*locations;

	locations = config.getLocMap("", request.port_number);
	if (locations->begin() == locations->end())
	{
		status_code = 404;
		return (locations);
	}
	matching_host = get_match_vect("", request, *locations, config);
	if (matching_host[0] == false)
		status_code = 404;
	return (locations);
}

std::string get_file_path(HttpRequest &request, Config &config, int &status_code)
{
	std::string file_path;
	unsigned int	i = 0;
	std::string	reqHost = request.headers["Host"];
	reqHost.erase(std::remove(reqHost.begin(), reqHost.end(), '\r'), reqHost.end());
	reqHost = reqHost.substr(0,reqHost.find(':'));
	request_ip_check(reqHost, config, status_code);
	if (status_code == 403)
		return ("");
	std::string tempHost = std::string(reqHost);
	std::map<std::string, LocationDir>	*locations = config.getLocMap(tempHost, request.port_number);
	std::map<std::string, LocationDir>::iterator	locEnd = locations->end();
	std::map<std::string, LocationDir>::iterator	it;
	if (locations->begin() == locEnd)
	{
		tempHost = "";
		locations = config.getLocMap(tempHost, request.port_number);
		if (locations->begin() == locations->end())
		{ status_code = 404; return (""); }
	}
	std::string	locRoute;
	size_t	slashPos = 1;
	size_t dotPos = NPOS;
	std::vector<bool>			matching_host;
	matching_host = get_match_vect(tempHost, request, *locations, config);
	if (matching_host[0] == false && tempHost != "")
	{
			tempHost = "";
			locations = getNullLocation(config, request, status_code, matching_host);
			if (status_code == 404)
				return("");
	}
	else if (matching_host[0] == false && tempHost == "")
		{ status_code = 404; return ("");}

	if (request.path.length() > 1)
		slashPos = request.path.find('/', 1);
	dotPos = request.path.find('.');
	if (dotPos != NPOS)
	{
		file_path = file_request_case(dotPos, request, config, status_code);
		if (status_code != 200 || !file_path.empty()) return (file_path);
	}
	unsigned int	host_index = 0;
	locEnd = locations->end();
	for (it = locations->begin(); it != locEnd; it++)
	{
		locRoute = it->first.substr(0, it->first.find(' '));
		if (matching_host[host_index + 1] == true && locRoute == request.path.substr(0, slashPos))
		{
			if (!(it->second.get_redir().empty()))
			{ status_code = 301; return (it->second.get_redir()); }
			break;
		}
		host_index++;
	}
	if (it == locEnd && tempHost != "")
	{
		locations = getNullLocation(config, request, status_code, matching_host);
		if (status_code == 404)
			return ("");
		host_index = 0;

		locEnd = locations->end();
		for (it = locations->begin(); it != locEnd; it++)
		{
			locRoute = it->first.substr(0, it->first.find(' '));
			if (matching_host[host_index + 1] == true && locRoute == request.path.substr(0, slashPos))
			{
				if (!(it->second.get_redir().empty()))
				{ status_code = 301; return (it->second.get_redir()); }
				break;
			}
			host_index++;
		}
		if (it == locEnd && locations->begin() != locEnd)
		{ status_code = 404; return (""); }
	}
	else if (it == locEnd && tempHost == "")
	{ status_code = 404; return (""); }
	std::vector<std::string> methods = it->second.get_methods_allowed();
	while (i < methods.size() && methods[i] != request.method)
		i++;
	if (i < methods.size())
		return (check_index_files(request, it, slashPos, dotPos, status_code));
	status_code = 405; return ("");
}

void	get_directory_listing(std::string & file_path, HttpRequest &request,
		std::vector<char>& body) {
	std::vector<std::string>::iterator	it;
	std::vector<std::string>			list;
	struct dirent						*en;
	DIR									*dir;

	dir = opendir(file_path.c_str());
	std::string reqHost = request.headers["Host"];
    reqHost.erase(std::remove(reqHost.begin(), reqHost.end(), '\r'), reqHost.end());
	reqHost.erase(std::remove(reqHost.begin(), reqHost.end(), ' '), reqHost.end());
	if (dir) {
		while ((en = readdir(dir)) != NULL) {
			list.push_back(en->d_name);
		}
		closedir(dir);
		body.insert(body.end(), START_OF_LIST.begin(), START_OF_LIST.end());
		std::string	line;

		for (it = list.begin(); it != list.end(); ++it) {
			line += "<li><a href=\"http://" + reqHost + request.path + "/" + *it + "\">" + *it + "</a></li>";
			body.insert(body.end(), line.begin(), line.end());
			line.clear();
		}
		body.insert(body.end(), END_OF_LIST.begin(), END_OF_LIST.end());
	} else {
		throw std::ios_base::failure("Error opening the directory!");
	}
}
