#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <deque>
#include "types/types.h"
#include "conf_parsing/Config.hpp"

class HttpResponse
{
    public:
        strstrMap	headers;
        std::string	method;
        std::string	path;
        std::string	http_version;
        std::string	body;
		int			status_code;
};

class ResponseFormatting
{
	public:
		static std::string	parse_headers(std::deque<std::string> &status_infos, std::string http_version,
						int const &status_code, Config &config, int content_length);
		static std::string	parse_cgi_headers(std::string http_version, 
		int content_length, int &status_code, std::deque<std::string> status_infos);
		static void	parse_body(std::string file_path, int const &status_code, std::vector<char>& body);
		static std::string	format_response(HttpRequest &request, int &status_code,
			std::string &file_path, Config &config, std::vector<char>& body);
};

std::deque<std::string>	get_status_infos(int status_code, std::string &file_path, std::string &error_path);
