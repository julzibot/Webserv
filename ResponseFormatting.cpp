#include "ResponseFormatting.hpp"

std::deque<std::string>	get_status_infos(int status_code, std::string &file_path, std::string const &error_path)
{
	std::deque<std::string>	status_infos;
	switch (status_code)
	{
		case 200:	status_infos.push_back(file_path); status_infos.push_back("OK");	 break;
		case 1001:	status_infos.push_back(file_path); status_infos.push_back("OK");	 break;
		case 301:	status_infos.push_back(file_path); status_infos.push_back("Moved Permanently"); 	break;
		case 400:	status_infos.push_back(error_path + "/400.html"); status_infos.push_back("Bad Request"); 	break;
		case 403:	status_infos.push_back(error_path + "/403.html"); status_infos.push_back("Forbidden"); 	break;
		case 404:	status_infos.push_back(error_path + "/404.html"); status_infos.push_back("Not Found"); 	break;
		case 405:	status_infos.push_back(error_path + "/405.html"); status_infos.push_back("Method Not Allowed"); break;
		case 408:	status_infos.push_back(error_path + "/408.html"); status_infos.push_back("Request Timeout"); break;
		case 413:	status_infos.push_back(error_path + "/413.html"); status_infos.push_back("Payload Too Large"); break;
		case 500:	status_infos.push_back(error_path + "/500.html"); status_infos.push_back("Internal Server Error");	break;
		case 504:	status_infos.push_back(error_path + "/504.html"); status_infos.push_back("Gateway Timeout"); 	break;
		// case 201:	status_infos.push_back(error_path); status_infos.push_back("Created"); 	break;
		// case 302:	status_infos.push_back(error_path); status_infos.push_back("Found"); 	break;
		// case 303:	status_infos.push_back(error_path); status_infos.push_back("See Other"); 	break;
		// case 401:	status_infos.push_back(error_path); status_infos.push_back("Unauthorized"); 	break;
		// case 501:	status_infos.push_back(error_path + "/501.html"); status_infos.push_back("Not Implemented"); 	break;
		// case 502:	status_infos.push_back(error_path); status_infos.push_back("Bad Gateway"); 	break;
		// case 503:	status_infos.push_back(error_path); status_infos.push_back("Service Unavailable"); 	break;
	}
	return (status_infos);
}

std::string	get_content_type(std::string file_path, Config &config)
{
	std::string	content_type;
	std::string	file_ext;

	file_ext = file_path.substr(file_path.find_last_of(".") + 1);
	content_type = config.get_type(file_ext);

	return (content_type);
}

std::string	ResponseFormatting::parse_headers(std::deque<std::string> &status_infos,
	std::string http_version, int const &status_code, Config &config,
	int content_length)
{
	std::string	headers;

	headers = http_version + " " + std::to_string(status_code) + " "
		+ status_infos[1] + "\n";
	if (status_code != 301 && status_code != 408)
	{
		headers += "Content-Type: " + get_content_type(status_infos[0], config) + "\n";
		headers += "Content-Length: " + std::to_string(content_length) + "\n";
	}
	else if (status_code == 301)
		headers += "Location: " + status_infos[0];
	else
		headers += "Connection: close";

	return (headers);
}

std::string	ResponseFormatting::parse_body(std::string file_path, int const &status_code)
{
	std::ifstream	inputFile(file_path);
	std::string		output;
	std::string		line;

	if (status_code == 301 || status_code == 408)
		return (output);
	if (!inputFile.is_open())
		return output;
	while (std::getline(inputFile, line))
		output += line + '\n';
	inputFile.close();
	return output;
}

std::string	ResponseFormatting::format_response(
	HttpRequest const &request, int &status_code, std::string &file_path,
	Config &config)
{
	std::string	output;
	std::string	body;
	std::string	headers;
	std::string p = request.path.substr(0, request.path.find('/', 1));
	std::deque<std::string> status_infos = get_status_infos(status_code,
			file_path, config.getServMain(request.port_number, p, true)["error_pages"]);

	if (status_code == 1001)
	{
		try {
			body = get_directory_listing(file_path, request, config);
			status_code = 200;
			headers = parse_headers(status_infos, request.http_version, status_code,
						config, body.length());
		} catch (const std::ios_base::failure& e) {
			status_code = 403;
			status_infos = get_status_infos(status_code,
				file_path, config.getServMain(request.port_number, p, true)["error_pages"]);
			body = parse_body(status_infos[0], status_code);
		}
	}
	else
	{
		body = parse_body(status_infos[0], status_code);
	headers = parse_headers(status_infos, request.http_version, status_code,
			config, body.length());
	}
	output = headers;
	if (body.length() > 0)
		output += '\n' + body;
	return (output);
}
