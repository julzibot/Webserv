/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2024/02/04 09:31:17 by mstojilj         ###   ########.fr       */
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
		std::cerr << RED << "Error: Bad number of arguments" << RESETCLR << std::endl;
		std::cerr << RED << "Only '.config' file is accepted" << RESETCLR << std::endl;
		return (1);
	}

	if (argc == 2)
		WebServ	server(argv[1], envp);
	else
		WebServ	server("server_files/configuration_files/tosh_tests.conf", envp);

    return (0);
}
