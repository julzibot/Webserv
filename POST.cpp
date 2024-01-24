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
	char	binChar[30];
	
	while (chunkSize > 0) {
		memset(binChar, 0, 2);
		chunkSize = recv(sockClient, binChar, 1, 0);
		if (chunkSize == 0)
			break;
		_binaryBody.push_back(binChar[0]);
	}
	std::vector<char>::iterator	boundIt = std::search(_binaryBody.begin(),
		_binaryBody.end(), endBoundary.begin(), endBoundary.end());
	if (boundIt != _binaryBody.end())
		_binaryBody.erase(boundIt, _binaryBody.end());
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

		formHeaderData.append(formDataBody);
		if (formHeaderData.find("\r\n\r\n") != NPOS) {
			receiveBinary(sockClient, "--" + boundary + "--");
			break;
		}
	}

	// parse form data:
	// - supported body type
	// - filename
	std::vector<std::string>	formHeaders;
	std::istringstream			formStream(formHeaderData);
	std::string					tmpLine;
	
	while (std::getline(formStream, tmpLine)) {
		tmpLine.erase(tmpLine.find("\r"));
		formHeaders.push_back(tmpLine);
	}
	formStream.clear();
	formHeaderData.clear();

	// Parse body form info
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
	std::string	filename = "/" + inputName;
	if (fileType == "image/jpeg")
		filename += ".jpg";
	else if (fileType == "image/png")
		filename += ".png";
	root += filename;
	std::ofstream	newFile;
	newFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	try {
		// Check if file exists
		struct stat	fileInfos;
		if (stat(root.c_str(), &fileInfos) != -1) {
			_status = 409; // Conflict
			return;
		}
		
		newFile.open(root, std::ios::binary);
		for (std::vector<char>::iterator it = _binaryBody.begin(); it != _binaryBody.end(); ++it)
			newFile << *it;
	}
	catch (const std::ofstream::failure& e) {
		std::cerr << RED << "Caught exception: receiveMultiForm(): '" << e.what()
			<< "' while receiving file on port: " << _request.port_number << RESETCLR << std::endl;
		_status = 500;
	}
	newFile.close();
	_binaryBody.clear();
	return;

	// // Create timestamp
	// struct timeval	currentTime;
	// if (gettimeofday(&currentTime, NULL) < 0)
	// 	printErrno(GETTIMEOFDAY, EXIT);

	// time_t		now = currentTime.tv_sec;
    // struct tm	*tm_now = localtime(&now);
    // char		date_buffer[20];

    // strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d %H:%M:%S", tm_now);

}

void	WebServ::receiveFileOnly(const int& sockClient, const std::string& fileType,
	const std::string& root) {

	char		binChar[2];
	int			chunkSize = 1;
	std::string	fileName;

	if (fileType == "image/jpeg")
		fileName = root + "/unknown_name.jpg";
	else if (fileType == "image/png")
		fileName = root + "/unknown_name.png";

	while (chunkSize > 0) {
		memset(binChar, 0, 2);
		chunkSize = recv(sockClient, binChar, 1, 0);
		if (chunkSize == 0)
			break;
		_binaryBody.push_back(binChar[0]);
	}
	std::ofstream	newFile;
	newFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	try {
		newFile.open(fileName, std::ios::binary);

		// if file exists, status code = 409 Conflict
		for (std::vector<char>::iterator it = _binaryBody.begin(); it != _binaryBody.end(); ++it)
			newFile << *it;
		newFile.close();
	}
	catch (const std::ofstream::failure& e) {
		newFile.close();
		std::cerr << RED << "Caught exception: receiveFileOnly(): '" << e.what()
			<< "' while receiving file on port: " << _request.port_number << RESETCLR << std::endl;
		_status = 500; // Internal Server Error
	}
	_binaryBody.clear();
	return;
}

void	WebServ::receiveBody( const int& sockClient ) {
	std::string p = _request.path.substr(0, _request.path.find('/', 1));
	std::string	root = _config.getServMain(_request.port_number, p, true)["root"];
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
	// if File or Form (Content-Type)
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
		receiveFileOnly(sockClient, fileType, root);
}
