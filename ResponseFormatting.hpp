/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseFormatting.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/01 19:01:48 by toshsharma        #+#    #+#             */
/*   Updated: 2024/01/11 00:14:13 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		static std::string	parse_body(std::string file_path, int const &status_code);
		static std::string	format_response(HttpRequest const &request, int &status_code,
						std::string &file_path, Config &config);
};
