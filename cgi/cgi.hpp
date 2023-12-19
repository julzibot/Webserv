/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/15 18:41:58 by toshsharma        #+#    #+#             */
/*   Updated: 2023/12/19 15:57:57 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include "../RequestParsing.hpp"

class CGI {
	private:
		std::string					cgi_path;
		std::vector<std::string>	cgi_args;
		std::vector<std::string>	cgi_envp;
	public:
		CGI(std::string cgi_path, std::vector<std::string> cgi_args, std::vector<std::string> get_envp);
		~CGI();
		std::string		get_cgi_path();
		char * const *	get_cgi_args();
		char * const *	get_envp();
};

std::string	execute_cgi(HttpRequest &request, CGI &cgi);