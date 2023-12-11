/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/01 19:07:18 by toshsharma        #+#    #+#             */
/*   Updated: 2023/12/10 23:55:25 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>

typedef std::map<std::string, std::string> strstrMap;
const std::string START_OF_LIST =
	"<!DOCTYPE html><html><head><title>Directory List</title></head><body>";
const std::string END_OF_LIST = "</body></html>";
