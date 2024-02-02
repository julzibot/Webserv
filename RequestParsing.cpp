#include "RequestParsing.hpp"
#include "WebServ.hpp"
#include "conf_parsing/Config.hpp"

HttpRequest::HttpRequest() : content_length(UINT_MAX), cgi(false), keepalive(true) {}

HttpRequest::HttpRequest(HttpRequest const &req) : headers(req.headers),
	method(req.method), path(req.path), http_version(req.http_version),
	port_number(req.port_number), cgi(false) {}

void    HttpRequestParse::parse_headers(std::istringstream &rs, HttpRequest &request)
{
	std::string line;
	std::string testline;

	while (std::getline(rs, line))
	{
		line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
		line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
		if (line.empty())
			return;

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

void	HttpRequestParse::parse(HttpRequest& request, std::vector<char> &req_str, int portnum)
{
	// Extracting headers from the entire request
	const char	*crlf = "\r\n\r\n";
	std::vector<char>::iterator	it = std::search(req_str.begin(), req_str.end(), crlf, crlf + 4);
	size_t	headerSize = std::distance(req_str.begin(), it);
	std::string	headers(headerSize, '\0');

	for (size_t j = 0; j < headerSize; j++)
		headers[j] = req_str[j];
	// WebServ::removeUntilCRLF(req_str);
	// for (size_t i = 0; i < req_str.size(); ++i)
	// 	std::cout << req_str[i];
	// std::cout << "*** *** ***" << std::endl;

	// Headers parse
	request.port_number = portnum;
	std::istringstream	requestStream(headers);
	std::string			line;
	std::getline(requestStream, line);
	std::istringstream	linestream(line);

	linestream >> request.method >> request.path >> request.http_version;
	HttpRequestParse::parse_headers(requestStream, request);
	strstrMap::iterator	headerIt = request.headers.find("Content-Length");
	if (headerIt != request.headers.end())
		request.content_length = std::atoi(headerIt->second.c_str());
	headerIt = request.headers.find("Connection");
	if (headerIt != request.headers.end() && headerIt->second == "close")
		request.keepalive = false;

	// Extract body from the entire request
	if (it != req_str.end() && headerSize + 4 < req_str.size())
	{
		std::cout << "PARSING BODY" << std::endl;
		it += 4;
		size_t	bodySize = req_str.size() - headerSize - 4;
		request.body.clear();
		request.body.resize(bodySize);
		request.body.assign(it, req_str.end());
	}
	// std::cout << "BODY" << std::endl;
	// for (size_t k = 0; k < request.body.size(); k++)
	// 	std::cout << request.body[k];
}
