#include "ResponseFormatting.hpp"
#include "conf_parsing/DirectiveParsing.h"

std::string	to_string(int number)
{
	std::stringstream ss;
	ss << number;
	std::string result = ss.str();
	return result;
}

std::deque<std::string>	get_status_infos(int status_code, std::string &file_path, std::string &error_path)
{
	if (error_path.empty())
		error_path = "./server_files/error_pages";
	std::deque<std::string>	status_infos;
	switch (status_code)
	{
		case 200:	status_infos.push_back(file_path); status_infos.push_back("OK");	 break;
		case 204:	status_infos.push_back(error_path + "/204.html"); status_infos.push_back("No Content"); 	break;
		case 301:	status_infos.push_back(file_path); status_infos.push_back("Moved Permanently"); 	break;
		case 400:	status_infos.push_back(error_path + "/400.html"); status_infos.push_back("Bad Request"); 	break;
		case 403:	status_infos.push_back(error_path + "/403.html"); status_infos.push_back("Forbidden"); 	break;
		case 404:	status_infos.push_back(error_path + "/404.html"); status_infos.push_back("Not Found"); 	break;
		case 405:	status_infos.push_back(error_path + "/405.html"); status_infos.push_back("Method Not Allowed"); break;
		case 408:	status_infos.push_back(error_path + "/408.html"); status_infos.push_back("Request Timeout"); break;
		case 409:	status_infos.push_back(error_path + "/409.html"); status_infos.push_back("Conflict"); break;
		case 413:	status_infos.push_back(error_path + "/413.html"); status_infos.push_back("Payload Too Large"); break;
		case 415:	status_infos.push_back(error_path + "/415.html"); status_infos.push_back("Unsupported Media Type"); break;
		case 422:	status_infos.push_back(error_path + "/422.html"); status_infos.push_back("Unprocessable Content"); break;
		case 500:	status_infos.push_back(error_path + "/500.html"); status_infos.push_back("Internal Server Error");	break;
		case 504:	status_infos.push_back(error_path + "/504.html"); status_infos.push_back("Gateway Timeout"); 	break;
		case 1001:	status_infos.push_back(file_path); status_infos.push_back("OK");	 break;
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
	if (content_type == "")
		content_type = "application/octet-stream";
	return (content_type);
}

std::string	ResponseFormatting::parse_headers(std::deque<std::string> &status_infos,
	std::string http_version, int const &status_code, Config &config,
	int content_length)
{
	std::string	headers;

	if (status_code == 1001) 
		headers = http_version + " 200 " + status_infos[1] + "\n";
	else
		headers = http_version + " " + to_string(status_code) + " " + status_infos[1] + "\n";	

	if (status_code == 1001)
	{
		headers += "Content-Type: text/html\n";
		headers += "Content-Length: " + to_string(content_length) + "\n";
	} else if (status_code != 301 && status_code != 408)
	{
		headers += "Content-Type: " + get_content_type(status_infos[0], config) + "\n";
		headers += "Content-Length: " + to_string(content_length) + "\n";
	}
	else if (status_code == 301)
		headers += "Location: " + status_infos[0] + "\nContent-Length: 0";
	else
		headers += "Connection: close";

	return (headers);
}

std::string	ResponseFormatting::parse_cgi_headers(std::string http_version, int content_length)
{
	std::string	headers;

	headers = http_version + "  200 OK"+ "\n";
	headers += "Content-Type: text/plain\n";
	headers += "Content-Length: " + to_string(content_length) + "\n";

	return (headers);
}

void	ResponseFormatting::parse_body(std::string file_path, int const &status_code, std::vector<char>& body)
{
	if (status_code == 301 || status_code == 408)
		return;

	std::ifstream	inputFile(file_path);
	std::string		line;

	if (!inputFile.is_open())
		return;
	
	if (file_path.find(".jpg") != NPOS || file_path.find(".ico") != NPOS) {

		std::ifstream	binaryFile(file_path, std::ios::binary);
		binaryFile.seekg(0, std::ios::end);

		std::streamsize	fileSize = binaryFile.tellg();

		body.resize(fileSize);
		binaryFile.seekg(0, std::ios::beg);

		if (!binaryFile.read(body.data(), fileSize)) {
			std::cerr << "parse_body(): Set status code to 500!" << std::endl;
			// status_code = 500; // Internal Server Error
		}
		binaryFile.close();
	}
	else {
		while (std::getline(inputFile, line)) {
			line += '\n';
			body.insert(body.end(), line.begin(), line.end());
		}
	}
	inputFile.close();
}

std::string	ResponseFormatting::format_response(HttpRequest &request, int &status_code,
		std::string &file_path, Config &config, std::vector<char>& body)
{
	std::string	output;
	std::string	headers;
	std::string p = request.path.substr(0, request.path.find('/', 1));
	std::string	reqHost = request.hostIP;

	std::deque<std::string> status_infos = get_status_infos(status_code,
			file_path, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);

	if (status_code == 1001)
	{
		try {
			get_directory_listing(file_path, request, config, body);
			headers = parse_headers(status_infos, request.http_version, status_code,
						config, body.size());
		} catch (const std::ios_base::failure& e) {
			status_code = 403;
			status_infos = get_status_infos(status_code,
				file_path, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);
			parse_body(status_infos[0], status_code, body);
		}
	}
	else
	{
		parse_body(status_infos[0], status_code, body);
		if (status_infos[0].find(".jpg") != NPOS || status_infos[0].find(".ico") != NPOS)
			headers = parse_headers(status_infos, request.http_version,
					status_code, config, body.size());
		else
		headers = parse_headers(status_infos, request.http_version,
					status_code, config, body.size());
	}
	output = headers + "\n";
	return (output);
}
