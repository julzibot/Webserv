/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/15 18:49:50 by toshsharma        #+#    #+#             */
/*   Updated: 2023/12/15 19:30:07 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cgi.hpp"

std::string	execute_cgi(HttpRequest &request, CGI &cgi)
{
	std::string	output;
	int			fd[2];
	pid_t		pid;
	int			status;

	if (pipe(fd) == -1)
		throw std::exception();
	pid = fork();
	if (pid == -1)
		throw std::exception();
	if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		if (execve(cgi.get_cgi_path().c_str(), cgi.get_cgi_args(), cgi.get_envp()) == -1)
			throw std::exception();
	}
	else
	{
		close(fd[1]);
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) == 0)
			{
				char buffer[1024];
				int bytes_read;
				while ((bytes_read = read(fd[0], buffer, 1024)) > 0)
					output.append(buffer, bytes_read);
			}
			else
				throw std::exception();
		}
		else
			throw std::exception();
	}

	return output;
}