/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mstojilj <mstojilj@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 22:39:27 by mstojilj          #+#    #+#             */
/*   Updated: 2023/12/22 15:19:09 by mstojilj         ###   ########.fr       */
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

int main(int argc, char **argv) {

	if (argc > 2) {
		std::cout << "Error: Bad number of arguments. Only '.config' file is accepted.";
		return (1);
	}

	if (argc == 2)
		WebServ	server(argv[1]);
	else
		WebServ	server("server_files/webserv.conf");

    return (0);
}
