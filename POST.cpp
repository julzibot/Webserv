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

void	WebServ::parseBinary(const std::string& endBoundary) {

	if (endBoundary != "") {
		std::vector<char>::iterator	boundIt = std::search(_request.body.begin(),
			_request.body.end(), endBoundary.begin(), endBoundary.end());
		if (boundIt != _request.body.end())
			_request.body.erase(boundIt, _request.body.end());
	}
}

void	WebServ::storeFile(const std::string& fileType, const std::string& filename,
	const std::string& root) {

	std::ofstream	newFile;
	std::string		filePath;

	struct stat rootStat;
	
	// check if root is valid/exists
	if (stat(root.c_str(), &rootStat) != 0) {
		_status = 500; // Internal Server Error
		std::cerr << RED << "Error: storeFile(): Invalid root" << RESETCLR << std::endl;
		return;
	}
	// // Create timestamp
	struct timeval	currentTime;
	if (gettimeofday(&currentTime, NULL) < 0)
		printErrno(GETTIMEOFDAY, EXIT);

	time_t		now = currentTime.tv_sec;
    struct tm	*tm_now = localtime(&now);
    char		timestamp[20];

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", tm_now);

	if (fileType == "image/jpeg")
		filePath = root + "/uploaded-files/" + filename + "_" + timestamp + ".jpg";
	else if (fileType == "image/png")
		filePath = root + "/uploaded-files/" + filename + "_" + timestamp + ".png";
	else
		filePath = root + "/uploaded-files/" + filename + "_" + timestamp;
	parseBinary("");

	if (_status == 200 && (_request.path.find("cgi")) == NPOS) {
		newFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try {

			struct stat	fileInfos;
			if (stat(filePath.c_str(), &fileInfos) != -1) {
				_status = 409; // Conflict
				_request.body.clear();
				return;
			}
			newFile.open(filePath.c_str(), std::ios::binary);
			for (std::vector<char>::iterator it = _request.body.begin(); it != _request.body.end(); ++it)
				newFile << *it;
		}
		catch (const std::ofstream::failure& e) {
			std::cerr << RED << "Error: storeFile() or receiveMultiFormData(): '" << e.what()
				<< "' while receiving file on port: " << _request.port_number << RESETCLR << std::endl;
			_status = 500; // Internal Server Error
		}
		newFile.close();
	}
	return;
}

void	WebServ::receiveMultiForm( std::string root, std::string boundary )
{
	const char					*crlf = "\r\n\r\n";
	std::vector<char>::iterator	it = std::search(_request.body.begin(), _request.body.end(), crlf, crlf + 4);
	size_t						headerSize = std::distance(_request.body.begin(), it + 1);
	std::string					formHeaderData(headerSize, '\0');

	if (headerSize + 4 < _request.body.size())
		it += 4;	
	formHeaderData.assign(_request.body.begin(), it);
	removeUntilCRLF(_request.body);
	if (!_request.body.empty())
		parseBinary("--" + boundary + "--");

	std::istringstream			formStream(formHeaderData);
	std::string					tmpLine;
	std::vector<std::string>	formHeaders;
	
	while (std::getline(formStream, tmpLine)) {
		if (tmpLine.find("\r") != NPOS)
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
				std::cerr << RED << "Error: Unsupported file type" << RESETCLR << std::endl;
				_status = 415; // Unsupported Media Type
				formHeaders.clear();
				return;
			}
		}
	}
	storeFile(fileType, inputName, root);
	return;
}

void	WebServ::receiveBody( void )
{
	std::string p = _request.path.substr(0, _request.path.find('/', 1));
	std::string	reqHost = _request.hostIP;
	std::string	root = _config.getServMain(reqHost, _request.port_number, p, true)["root"];
	if (root.empty()) {
		std::cerr << RED << "Error: receiveBody(): 'root' not found" << RESETCLR << std::endl;
		_status = 500; // 500 Internal Server Error
		return;
	}

	std::string			fileType;
	std::string			fileName;
	strstrMap::iterator contentTypeIt = _request.headers.find("Content-Type");
	if (contentTypeIt == _request.headers.end()) {
		std::cerr << RED << "Error: POST: No 'Content-Type' in request headers" << RESETCLR << std::endl;
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
			std::cerr << RED << "Error: POST: No boundary inside multiform/data." << RESETCLR << std::endl;
			_status = 400; // Bad request
			return;
		}
		receiveMultiForm(root, boundary);
	}
	else
		storeFile(fileType, "unknown_name", root);
}
