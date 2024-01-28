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
		if (chunkSize == 0)
			break;
		// if (select(_maxFD + 1, &_readySockets, NULL, NULL, NULL) < 0)
		// 	printErrno(SELECT, EXIT);
		for (int i = 0; i < chunkSize + 1; ++i)
			_request._binaryBody.push_back(binChar[i]);
	}

	if (endBoundary != "") {
		std::vector<char>::iterator	boundIt = std::search(_request._binaryBody.begin(),
			_request._binaryBody.end(), endBoundary.begin(), endBoundary.end());
		if (boundIt != _request._binaryBody.end())
			_request._binaryBody.erase(boundIt, _request._binaryBody.end());
	}
	_request._bodyString.assign(_request._binaryBody.begin(), _request._binaryBody.end());
}

void	WebServ::receiveFile(const int& sockClient, const std::string& fileType, const std::string& filename,
	const std::string& root) {

	std::ofstream	newFile;
	std::string		filePath;

	if (fileType == "image/jpeg")
		filePath = root + "/" + filename + ".jpg";
	else if (fileType == "image/png")
		filePath = root + "/" + filename + ".png";
	else
		filePath = root + "/" + "unknown_file";
	receiveBinary(sockClient, "");

	newFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	try {

		struct stat	fileInfos;
		if (stat(filePath.c_str(), &fileInfos) != -1) {
			_status = 409; // Conflict
			_request._binaryBody.clear();
			return;
		}
		newFile.open(filePath, std::ios::binary);
		for (std::vector<char>::iterator it = _request._binaryBody.begin(); it != _request._binaryBody.end(); ++it)
			newFile << *it;
	}
	catch (const std::ofstream::failure& e) {
		std::cerr << RED << "Caught exception: receiveFileOnly() or receiveMultiFormData(): '" << e.what()
			<< "' while receiving file on port: " << _request.port_number << RESETCLR << std::endl;
		_status = 500; // Internal Server Error
	}
	newFile.close();
	_request._binaryBody.clear();
	return;
}

void	WebServ::receiveMultiForm( const int& sockClient, std::string root, std::string boundary ) {

	int			chunkSize = 1;
	char		formDataBody[2];
	std::string	formHeaderData;

	while (chunkSize > 0) {
		memset(formDataBody, 0, 2);
		chunkSize = recv(sockClient, formDataBody, 1, 0);
		if (chunkSize == 0)
			break;
		// if (select(_maxFD + 1, &_readySockets, NULL, NULL, &_timeoutSelect) < 0)
		// 	printErrno(SELECT, EXIT);
		formHeaderData.append(formDataBody);
		if (formHeaderData.find("\r\n\r\n") != NPOS) {
			receiveBinary(sockClient, "--" + boundary + "--");
			break;
		}
	}

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
