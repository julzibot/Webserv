/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParsing.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:37:50 by mstojilj          #+#    #+#             */
/*   Updated: 2024/01/17 15:52:38 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParsing.hpp"
#include "WebServ.hpp"
#include "conf_parsing/Config.hpp"

HttpRequest::HttpRequest() : cgi(false) {}

HttpRequest::HttpRequest(HttpRequest const &req) : headers(req.headers),
	method(req.method), path(req.path), http_version(req.http_version),
	body(req.body), port_number(req.port_number), cgi(false) {}

void    HttpRequestParse::parse_headers(std::istringstream &rs, HttpRequest &request)
{
	std::string line;
	while (std::getline(rs, line) && !line.empty())
	{
		// line.erase(line.find("\r"));
		size_t sepPos = line.find(":");
		if (sepPos != std::string::npos)
		{
			std::string headername = line.substr(0, sepPos);
			std::string headervalue = line.substr(sepPos + 2);
			headervalue.erase(std::find(headervalue.begin(), headervalue.end(), '\r'));
			request.headers[headername] = headervalue;
		}
		else
			break;
	}

}

HttpRequest	HttpRequestParse::parse(std::string const &req_str, int portnum)
{
	HttpRequest request;

	request.port_number = portnum;
	std::istringstream requestStream(req_str);
	std::string line;
	std::getline(requestStream, line);

	// PARSING START LINE
	std::istringstream linestream(line);
	linestream >> request.method >> request.path >> request.http_version;
	// TODO: 
	// CHECK REQUEST.METHOD IF IT IS ALLOWED METHOD
	
	// std::cout << request.method << request.path << request.http_version << std::endl;
	// PARSING HEADERS
	HttpRequestParse::parse_headers(requestStream, request);

    return (request);
}
