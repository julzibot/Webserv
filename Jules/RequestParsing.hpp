#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>


// parsing OUTPUT
class HttpRequest
{
    public:
        std::string method;
        std::string path;
        std::string http_version;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
};

class HttpRequestParse
{
    public:
        static HttpRequest parse(std::string const &req_str);
        static void parse_headers(std::istringstream &requestStream, HttpRequest &request);
};