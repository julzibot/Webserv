/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/01 19:07:18 by toshsharma        #+#    #+#             */
/*   Updated: 2024/02/01 09:35:59 by julzibot         ###   ########.fr       */
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
