#include "ResponseFormatting.hpp"
#include "conf_parsing/Config.hpp"
#include "conf_parsing/DirectiveParsing.h"

std::deque<std::string>	get_status_infos(int status_code, std::string &file_path, std::string &error_path)
{
	if (error_path.empty() || access(file_path.c_str(), F_OK) != 0)
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
		case 1001:	status_infos.push_back(file_path); status_infos.push_back("OK");	 break;
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

	if (status_code == 204)
		headers += "Content-Length: " + to_string(content_length) + "\n";
	else if (status_code == 1001)
	{
		headers += "Content-Type: text/html\n";
		headers += "Content-Length: " + to_string(content_length) + "\n";
	} else if (status_code != 301 && status_code != 408)
	{
		headers += "Content-Type: " + get_content_type(status_infos[0], config) + "\n";
		headers += "Content-Length: " + to_string(content_length) + "\n";
	}
	else if (status_code == 301)
		headers += "Location: " + status_infos[0] + "\nContent-Length: 0\n";
	else
		headers += "Connection: close";

	return (headers);
}

std::string	ResponseFormatting::parse_cgi_headers(std::string http_version, 
		int content_length, int &status_code, std::deque<std::string> status_infos)
{
	std::string	headers;

	if (status_code == 200) {
		headers = http_version + " 200 OK" + "\n";
		headers += "Content-Type: text/plain\n";
		headers += "Content-Length: " + to_string(content_length) + "\n";
	}
	else {
		headers = http_version + " " + to_string(status_code) + " " + status_infos[1] + "\n";
		headers += "Content-Type: text/html\n";
		headers += "Content-Length: " + to_string(content_length) + "\n";
	}
	return (headers);
}

void	ResponseFormatting::parse_body(std::string file_path, int &status_code, std::vector<char>& body)
{
	std::cout << "parsebody(): " << status_code << std::endl;
	std::cout << "parsebody() filepath: " << file_path << std::endl;
	if (status_code == 204 || status_code == 301) // || status_code == 408
		return;

	if (file_path.find(".jpg") != NPOS || file_path.find(".png")
		|| file_path.find(".ico") != NPOS)
	{
		std::ifstream	binaryFile(file_path.c_str(), std::ios::binary);

		if (!binaryFile.is_open())
			return;
		binaryFile.seekg(0, std::ios::end);

		std::streamsize	fileSize = binaryFile.tellg();

		body.resize(fileSize);
		binaryFile.seekg(0, std::ios::beg);

		if (!binaryFile.read(body.data(), fileSize)) {
			status_code = 500;
		}
		binaryFile.close();
	}
	else {
		std::cout << "408 file path: " << file_path.c_str() << std::endl;
		std::ifstream	inputFile(file_path.c_str());
		if (!inputFile.is_open())
			return;
		
		std::string		line;
		while (std::getline(inputFile, line)) {
			line += '\n';
			body.insert(body.end(), line.begin(), line.end());
		}
		std::cout << "body size: " << body.size() << std::endl;
		inputFile.close();
	}
}

std::string	ResponseFormatting::format_response(HttpRequest &request, int &status_code,
		std::string &file_path, Config &config, std::vector<char>& body)
{
	std::string	headers;
	std::string p = request.path.substr(0, request.path.find('/', 1));
	std::string	reqHost = request.hostIP;

	std::deque<std::string> status_infos = get_status_infos(status_code,
			file_path, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);

	if (status_code == 1001)
	{
		try {
			get_directory_listing(file_path, request, body);
			headers = parse_headers(status_infos, request.http_version, status_code,
						config, body.size());
		} catch (const std::ios_base::failure& e) {
			status_code = 403;
			status_infos = get_status_infos(status_code,
				file_path, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);
			parse_body(status_infos[0], status_code, body);
		}
	}
	else if (status_code == 204) {
		headers = parse_headers(status_infos, request.http_version,
					status_code, config, 0);
	}
	else
	{
		parse_body(status_infos[0], status_code, body);
		if (status_infos[0].find(".jpg") != NPOS || status_infos[0].find(".png") != NPOS
			|| status_infos[0].find(".ico") != NPOS)
			headers = parse_headers(status_infos, request.http_version,
					status_code, config, body.size());
		else
			headers = parse_headers(status_infos, request.http_version,
					status_code, config, body.size());
	}
	headers += "\n";
	return (headers);
}
