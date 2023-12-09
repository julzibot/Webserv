/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseFormatting.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/01 19:01:48 by toshsharma        #+#    #+#             */
/*   Updated: 2023/12/09 15:58:06 by julzibot         ###   ########.fr       */
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
		std::string	parse_headers(std::vector<std::string> &status_infos, std::string http_version,
						int const &status_code, Config &config, int content_length);
		std::string	parse_body(std::string file_path, int const &status_code);
		std::string	format_response(std::string http_version, int &status_code,
						std::string &file_path, Config &config);
};