#include "WebServ.hpp"

std::string	extractBetweenChars(std::string str, char c) {

	if (str.empty())
		return ("");
	std::string::size_type	start = str.find(c);
	if (start == std::string::npos)
		return ("");
	++start;
	std::string::size_type	end = start;

	while (end != str.length()) {
		if (str.at(end) == c)
			break;
		++end;
	}
	if (end == str.length() || start == end - 1)
		return ("");
	return (str.substr(start, end - start));
}

void	WebServ::receiveBinary(const int& sockClient, const std::string& endBoundary) {

	int		chunkSize = 1;
	char	binChar[4096];

	while (chunkSize > 0) {
		memset(binChar, 0, 4096);
		chunkSize = recv(sockClient, binChar, 4095, 0);
		if (chunkSize <= 0)
			break;
		for (int i = 0; i < chunkSize; ++i)
			_request.body.push_back(binChar[i]);
	}

	if (endBoundary != "") {
		std::vector<char>::iterator	boundIt = std::search(_request.body.begin(),
			_request.body.end(), endBoundary.begin(), endBoundary.end());
		if (boundIt != _request.body.end())
			_request.body.erase(boundIt, _request.body.end());
	}
}

void	WebServ::receiveFile(const int& sockClient, const std::string& fileType, const std::string& filename,
	const std::string& root) {

	std::ofstream	newFile;
	std::string		filePath;

	(void) sockClient;
	struct stat rootStat;
	
	// check if root is valid/exists
	if (stat(root.c_str(), &rootStat) != 0) {
		_status = 500; // Internal Server Error
		return;
	}
	// // Create timestamp
	struct timeval	currentTime;
	if (gettimeofday(&currentTime, NULL) < 0)
		printErrno(GETTIMEOFDAY, EXIT);

	time_t		now = currentTime.tv_sec;
    struct tm	*tm_now = localtime(&now);
    char		date_buffer[20];

    strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d %H:%M:%S", tm_now);

	if (fileType == "image/jpeg")
		filePath = root + "/" + filename + date_buffer + ".jpg";
	else if (fileType == "image/png")
		filePath = root + "/" + filename + date_buffer + ".png";
	else
		filePath = root + "/" + filename + "_" + date_buffer;
	// receiveBinary(sockClient, "");

	if (_status == 200 && (_request.path.find("cgi")) == NPOS) {
		newFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try {

			struct stat	fileInfos;
			if (stat(filePath.c_str(), &fileInfos) != -1) {
				_status = 409; // Conflict
				_request.body.clear();
				return;
			}
			newFile.open(filePath, std::ios::binary);
			for (std::vector<char>::iterator it = _request.body.begin(); it != _request.body.end(); ++it)
				newFile << *it;
		}
		catch (const std::ofstream::failure& e) {
			std::cerr << RED << "Caught exception: receiveFileOnly() or receiveMultiFormData(): '" << e.what()
				<< "' while receiving file on port: " << _request.port_number << RESETCLR << std::endl;
			_status = 500; // Internal Server Error
		}
		newFile.close();
	}
	return;
}

void	WebServ::receiveMultiForm( const int& sockClient, std::string root, std::string boundary )
{
	const char	*crlf = "\r\n\r\n";
	std::vector<char>::iterator	it = std::search(_request.body.begin(), _request.body.end(), crlf, crlf + 4);
	size_t	headerSize = std::distance(_request.body.begin(), it);
	std::string	formHeaderData(headerSize, '\0');

	if (headerSize + 4 < _request.body.size())
		formHeaderData.insert(formHeaderData.end(), _request.body.begin(), it + 4);

	if (formHeaderData.find("\r\n\r\n") != NPOS)
		receiveBinary(sockClient, "--" + boundary + "--");

	std::istringstream			formStream(formHeaderData);
	std::string					tmpLine;
	std::vector<std::string>	formHeaders;
	
	while (std::getline(formStream, tmpLine)) {
		tmpLine.erase(tmpLine.find("\r"));
		formHeaders.push_back(tmpLine);
	}
	formStream.clear();
	formHeaderData.clear();

	std::string	inputName;
	std::string	fileExt;
	std::string	fileType;

	for (size_t i = 0; i < formHeaders.size(); ++i) {

		if (formHeaders[i].find(" name=\"") != NPOS)
			inputName = extractBetweenChars(formHeaders[i].substr(formHeaders[i].find(" name=\"")), '\"');
		if (formHeaders[i].find(" filename=\"") != NPOS)
			fileExt = extractBetweenChars(formHeaders[i].substr(formHeaders[i].find(" filename=\"")), '\"');
		if (formHeaders[i].find("Content-Type: ") != NPOS) {
			fileType = formHeaders[i].substr(formHeaders[i].find("Content-Type: ") + 14);
			if (fileType != "image/jpeg" && fileType != "image/png")
			{
				std::cerr << "Error: Unsupported file type" << std::endl;
				_status = 415; // Unsupported Media Type
				formHeaders.clear();
				return;
			}
		}
	}
	receiveFile(sockClient, fileType, inputName, root);
	return;
}

void	WebServ::receiveBody( const int& sockClient ) {

	std::string p = _request.path.substr(0, _request.path.find('/', 1));
	std::string	reqHost = _request.hostIP;
	std::string	root = _config.getServMain(reqHost, _request.port_number, p, true)["root"];
	if (root.empty()) {
		std::cerr << "receiveBody: 'root' not found" << std::endl;
		_status = 500; // 500 Internal Server Error
		return;
	}

	std::string			fileType;
	std::string			fileName;
	strstrMap::iterator contentTypeIt = _request.headers.find("Content-Type");
	if (contentTypeIt == _request.headers.end()) {
		std::cerr << "Error: Cannot process POST request: No 'Content-Type' in request headers" << std::endl;
		_status = 422; // Unprocessable Content
		return;
	}
	fileType = contentTypeIt->second;
	if (fileType.find("multipart/form-data") != NPOS) {
		std::string	boundary;
		try {
			boundary.append(fileType.substr(fileType.find("boundary=") + 9, fileType.length() - 1));
		}
		catch (std::exception &e) {
			std::cerr << "Error: Cannot process POST request: No boundary inside multiform/data." << std::endl;
			_status = 400; // Bad request
			return;
		}
		receiveMultiForm(sockClient, root, boundary);
	}
	else
		receiveFile(sockClient, fileType, "unknown_name", root);
}
