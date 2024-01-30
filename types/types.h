/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/01 19:07:18 by toshsharma        #+#    #+#             */
/*   Updated: 2024/01/29 21:38:46 by mstojilj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>

typedef std::map<std::string, std::string> strstrMap;
const std::string START_OF_LIST =
	"<!DOCTYPE html><html><head><title>Directory List</title></head><body><h1>Directory listing</h1><ul>";
const std::string END_OF_LIST = "</ul></body></html>";
