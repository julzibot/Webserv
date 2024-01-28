#include "ResponseFormatting.hpp"
#include "conf_parsing/DirectiveParsing.h"

std::string	to_string(int number)
{
	std::stringstream ss;
	ss << number;
	std::string result = ss.str();
	return result;
}

std::deque<std::string>	ResponseFormatting::get_status_infos(int status_code, std::string &file_path, std::string &error_path)
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

// std::string	ResponseFormatting::parse_body(std::string file_path, int const &status_code)
// {
// 	std::ifstream	inputFile(file_path, std::ios::binary);
// 	std::string		output;
// 	std::string		line;

// 	if (status_code == 301) //  || status_code == 408)
// 		return (output);
// 	if (!inputFile.is_open())
// 		return output;

//     std::vector<char> buffer(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
//     return std::string(buffer.begin(), buffer.end());
// }

std::string	ResponseFormatting::parse_body(std::string file_path, int const &status_code)
{
    std::ifstream	inputFile(file_path.c_str(), std::ios::binary);
    std::string		output;
    std::string		line;

    if (status_code == 301) //  || status_code == 408)
        return (output);
    if (!inputFile.is_open())
        return output;

    inputFile.seekg(0, std::ios::end);
    std::streamsize size = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    if (size > 0)
    {
        output.resize(size);
        inputFile.read(&output[0], size);
    }

    return output;
}

std::string	ResponseFormatting::format_response(HttpRequest &request, int &status_code,
		std::string &file_path, Config &config)
{
	std::string	output;
	std::string	body;
	std::string	headers;
	std::string p = request.path.substr(0, request.path.find('/', 1));
	std::string	reqHost = request.hostIP;

	std::deque<std::string> status_infos = get_status_infos(status_code,
			file_path, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);

	if (status_code == 1001)
	{
		try {
			body = get_directory_listing(file_path, request, config);
			headers = parse_headers(status_infos, request.http_version, status_code,
						config, body.length());
		} catch (const std::ios_base::failure& e) {
			status_code = 403;
			status_infos = get_status_infos(status_code,
				file_path, config.getServMain(reqHost, request.port_number, p, true)["error_pages"]);
			body = parse_body(status_infos[0], status_code);
		}
	}
	else
	{
		body = parse_body(status_infos[0], status_code);
		headers = parse_headers(status_infos, request.http_version,
				status_code, config, body.length());
	}
	output = headers;
	// if (body.length() > 0)
	output += '\n' + body + '\n';
	return (output);
}
