/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:37:45 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/26 12:21:57 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

// parsing input on the server side
class HttpRequest
{
    public:
        std::map<std::string, std::string>	headers;
        std::string                         hostIP;
        std::string							method;
        std::string							path;
        std::string							http_version;
        std::string							body;
        std::string                         prio_file;
		int									port_number;
		unsigned int						content_length;
        bool                                cgi;
		bool								keepalive;
		std::vector<char>					_binaryBody;
		std::string							_bodyString;

        HttpRequest();
        HttpRequest(HttpRequest const &req);
};

class HttpRequestParse
{
    public:
        char *  process_request(char *buff, int recvsize, int port_number);
        static HttpRequest parse(std::string const &req_str, int portnum);
        static void parse_headers(std::istringstream &requestStream, HttpRequest &request);
};
