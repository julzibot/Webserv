/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julzibot <julzibot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2024/02/01 09:32:44 by julzibot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "RequestParsing.hpp"
#include "ResponseFormatting.hpp"
#include "conf_parsing/Config.hpp"
#include "conf_parsing/DirectiveParsing.h"
#include <csignal>
#include <fcntl.h>
#include <sys/time.h>

int main(int argc, char **argv, char **envp) {

	if (argc > 2) {
		std::cout << "Error: Bad number of arguments. Only '.config' file is accepted.";
		return (1);
	}

	if (argc == 2)
		WebServ	server(argv[1], envp);
	else
		WebServ	server("server_files/configuration_files/webserv.conf", envp);

    return (0);
}
