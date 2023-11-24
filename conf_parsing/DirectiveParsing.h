/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveParsing.h                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 15:39:48 by mstojilj          #+#    #+#             */
/*   Updated: 2023/11/24 22:11:30 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "ConfigParse.hpp"

typedef	void	(*funcPtr)(ConfigParse& config, std::string line);

void		parseDirective(std::string line, std::string directive,
			ConfigParse& config);
void		initDirMap(std::map<std::string, funcPtr>& dirCase);
void		dirParseMain(ConfigParse& config, std::string line);
void		dirParseTypes(ConfigParse& config, std::string line);
// void	dirParseHttp(ConfigParse& config, std::string line);
void		dirParseEvents(ConfigParse& config, std::string line);
std::string	removeSpaces( std::string line );
void		dirParseLocation(int port, std::string route,
	std::string line, ConfigParse &config);