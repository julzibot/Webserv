/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/15 18:41:58 by toshsharma        #+#    #+#             */
/*   Updated: 2024/01/29 14:03:31 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include "../RequestParsing.hpp"
#include "../conf_parsing/Config.hpp"

class CGI {
	private:
		std::string			cgi_path;
		std::vector<char *>	cgi_args;
		std::vector<char *>	cgi_envp;
		std::string			root;
	public:
		CGI(char ** cgi_env, std::string &executable);
		~CGI();
		std::string		get_cgi_path();
		char **	get_cgi_args();
		char **	get_envp();
		void			insert_arg(std::string & arg);
		std::string&	get_root();
		void		set_root(std::string & root);
		std::string	execute_cgi(HttpRequest &request, CGI *cgi, std::string filepath, int &status_code);
};
