/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:37:45 by mstojilj          #+#    #+#             */
/*   Updated: 2024/02/04 15:28:42 by toshsharma       ###   ########.fr       */
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
        std::string                         prio_file;
		int									port_number;
		int									content_length;
        bool                                cgi;
		bool								keepalive;
		std::vector<char>					body;
		// std::vector<char>					binaryBody;
		std::vector<char>					fullRequest;
		std::string							strHeaders;
        bool                                accepted_method;

        HttpRequest();
        HttpRequest(HttpRequest const &req);
};

class HttpRequestParse
{
    public:
        char *  process_request(char *buff, int recvsize, int port_number);
        static void	parse(HttpRequest& request, std::vector<char> &req_str, int portnum);
        static void parse_headers(std::istringstream &requestStream, HttpRequest &request);
};
