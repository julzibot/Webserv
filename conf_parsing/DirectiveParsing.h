/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.h                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:39:48 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/28 12:39:36 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Config.hpp"

typedef	void	(*funcPtr)(Config& config, std::string line);

void		parseDirective(std::string line, std::string directive,
			Config& config);
void		initDirMap(std::map<std::string, funcPtr>& dirCase);
void		dirParseMain(Config& config, std::string line);
void		dirParseTypes(Config& config, std::string line);
// void	dirParseHttp(ConfigParse& config, std::string line);
void		dirParseEvents(Config& config, std::string line);
std::string	removeSpaces( std::string line );
void		auth_except(LocationDir& ld, std::string line);
void		dirParseLocation(int port, std::string route,
	std::string line, Config &config);