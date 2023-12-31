/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.h                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:39:48 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/15 17:37:01 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
std::string	get_directory_listing(std::string & file_path, HttpRequest const &request,
				Config& config);

/* Helper functions */
std::string	removeSpaces( std::string line );
void		auth_except(LocationDir& ld, std::string line);
bool		isValidErrCode( const std::string& errCode );
bool		validErrorHtmlFile( std::string filename );
void		assign_autoindex(LocationDir& ld, std::string value);
