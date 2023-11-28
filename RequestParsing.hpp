/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:37:45 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/28 12:39:16 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>

// parsing input on the server side
class HttpRequest
{
    public:
        std::map<std::string, std::string>	headers;
        std::string							method;
        std::string							path;
        std::string							http_version;
        std::string							body;
};

class HttpRequestParse
{
    public:
        char *  process_request(char *buff, int recvsize);
        static HttpRequest parse(std::string const &req_str);
        static void parse_headers(std::istringstream &requestStream, HttpRequest &request);
};