#pragma once
#include "Config.hpp"
#include <dirent.h>
#include "../types/types.h"

typedef	void	(*funcPtr)(Config& config, std::string line, std::string directive);

void	parseDirective(std::string line, std::string directive, Config& config);
void	initDirMap(std::map<std::string, funcPtr>& dirCase);
void	dirParseMain(Config& config, std::string line, std::string directive);
void	dirParseTypes(Config& config, std::string line, std::string directive);
void	dirParseEvents(Config& config, std::string line, std::string directive);
void	dirParseServer(Config& config, std::string line, std::string directive);
void	dirParseLocation(std::string line, Config &config, std::string directive);
std::string	get_directory_listing(std::string & file_path, HttpRequest &request,
				Config& config);

/* Helper functions */
std::string	removeSpaces( std::string line );
void		auth_except(LocationDir& ld, std::string line);
bool		isValidErrCode( const std::string& errCode );
bool		validErrorHtmlFile( std::string filename );
void		assign_autoindex(LocationDir& ld, std::string value);
bool		is_valid_IP(std::string const &str);
void	request_ip_check(std::string &reqHost, Config &config, int &status_code);
