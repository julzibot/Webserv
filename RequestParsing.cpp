#include "RequestParsing.hpp"
#include "conf_parsing/ConfigParse.hpp"

void    HttpRequestParse::parse_headers(std::istringstream &rs, HttpRequest &request)
{
    std::string line;
    while (std::getline(rs, line) && !line.empty())
    {
        size_t sepPos = line.find(":");
        if (sepPos != std::string::npos)
        {
            std::string headername = line.substr(0, sepPos);
            std::string headervalue = line.substr(sepPos + 1);
            request.headers[headername] = headervalue;
        }
        else
            throw std::exception();
    }
}

char *  HttpRequestParse::process_request(char * buffer, int recvsize)
{
    HttpRequestParse request;
    HttpRequest requestConfig;
    std::string file_path;
    char        *output;

    requestConfig = HttpRequestParse::parse(std::string(buffer, 0, recvsize));
    // file_path = config.get_file_path(requestConfig);
    /**
     * 1. check if file exists
     * 2. check if file is readable
     * 3. Read file.
     * 4. Convert into necessary format and respond.
    */
    return (output);
}

HttpRequest HttpRequestParse::parse(std::string const &req_str)
{
    HttpRequest request;

    std::istringstream requestStream(req_str);
    std::string line;
    std::getline(requestStream, line);

    // PARSING START LINE
    std::istringstream linestream(line);
    linestream >> request.method >> request.path >> request.http_version;
    std::cout << request.method << request.path << request.http_version;
    // PARSING HEADERS
    HttpRequestParse::parse_headers(requestStream, request);
    // PARSING BODY IF NECESSARY
    if (std::getline(requestStream, line) && !line.empty())
        request.body = line;

    // UNCOMMENT BELOW TO TEST PARSING RESULT
    // std::cout << "method: " << request.method << " path: " << request.path << " version: " << request.http_version << std::endl;
    // for (const auto& header : request.headers)
    //     std::cout << "Header: " << header.first << " = " << header.second << std::endl;
    // std::cout << "body: " << request.body << std::endl;

    return (request);
}
