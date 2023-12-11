#include "ResponseFormatting.hpp"

std::string	get_status_message(int status_code)
{
	std::string	status_message;

	switch (status_code) {
		case 200:	status_message = "OK";	break;
		case 201:	status_message = "Created";	break;
		case 301:	status_message = "Moved Permanently";	break;
		case 302:	status_message = "Found";	break;
		case 303:	status_message = "See Other";	break;
		case 400:	status_message = "Bad Request";	break;
		case 401:	status_message = "Unauthorized";	break;
		case 403:	status_message = "Forbidden";	break;
		case 404:	status_message = "Not Found";	break;
		case 500:	status_message = "Internal Server Error";	break;
		case 501:	status_message = "Not Implemented";	break;
		case 502:	status_message = "Bad Gateway";	break;
		case 503:	status_message = "Service Unavailable";	break;
		case 504:	status_message = "Gateway Timeout";	break;
	}
	return (status_message);
}

std::string	get_content_type(std::string file_path, Config &config)
{
	std::string	content_type;
	std::string	file_ext;

	std::cout << "FILEPATH: " << file_path << std::endl;
	file_ext = file_path.substr(file_path.find_last_of(".") + 1);
	content_type = config.get_type(file_ext);

	std::cout << "FILE EXT: " << file_ext << " CONT TYPE: "<< content_type << std::endl;
	return (content_type);
}

std::string	ResponseFormatting::parse_headers(std::string file_path,
	std::string http_version, int status_code, Config &config,
	int content_length)
{
	std::string	headers;

	headers = http_version + " " + std::to_string(status_code) + " "
		+ get_status_message(status_code) + "\n";
	// headers += "Content-Type: " + get_content_type(file_path, config) + "\n";
	headers += "Content-Type: text/html\n";
	headers += "Content-Length: " + std::to_string(content_length) + "\n";
	// headers += "Connection: close\n";

	return (headers);
}

std::string	ResponseFormatting::parse_body(std::string file_path)
{
	std::ifstream	inputFile(file_path);
	std::string		output;
	std::string		line;

	output = "";
	if (!inputFile.is_open())
		return output;
	while (std::getline(inputFile, line))
		output += line + '\n';
	inputFile.close();
	return output;
}

std::string	ResponseFormatting::format_response(
	std::string http_version, int status_code, std::string file_path,
	Config &config)
{
	std::string	output;
	std::string	body;
	std::string	headers;

	// body = parse_body(file_path);
	body = file_path;
	headers = parse_headers(file_path, http_version, status_code,
			config, body.length());
	output = headers + "\n" + body;
	return (output);
}
