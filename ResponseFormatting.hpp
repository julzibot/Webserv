/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseFormatting.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/01 19:01:48 by toshsharma        #+#    #+#             */
/*   Updated: 2023/12/06 16:56:21 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
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
		std::string	parse_headers(std::string file_path, std::string http_version,
						int status_code, Config &config, int content_length);
		std::string	parse_body(std::string file_path);
		std::string	format_response(std::string http_version, int status_code,
						std::string file_path, Config &config);
};