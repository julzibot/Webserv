/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toshsharma <toshsharma@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/15 18:49:50 by toshsharma        #+#    #+#             */
/*   Updated: 2023/12/20 18:54:10 by toshsharma       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

std::string	execute_cgi(HttpRequest &request, CGI &cgi)
{
	std::string	output;
	int			fd[2];
	pid_t		pid;

	(void)request;
	std::string file_to_exec = "/Users/toshsharma/Documents/42cursus/Webserv/server_files/cgi-bin/sample.py";
	cgi.insert_arg(file_to_exec);
	if (pipe(fd) == -1)
		throw std::exception();
	pid = fork();
	if (pid == -1)
		throw std::exception();
	if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		if (execve(cgi.get_cgi_path().c_str(), cgi.get_cgi_args(), cgi.get_envp()) == -1)
		{
			throw std::exception();
		}
	}
	else
	{
		close(fd[1]);
		waitpid(0, NULL, WNOHANG);
		char buffer[1024];
		int bytes_read;
		bytes_read = read(fd[0], buffer, 1024);
		output.append(buffer, bytes_read);
		while ((bytes_read = read(fd[0], buffer, 1024)) > 0)
		{
			output.append(buffer, bytes_read);
		}
		close(fd[0]);
	}
	return output;
}

/**
 * Since we need the root, we need to get the root from the config file
 * The root will be dependant on the request that comes in to the code
 * We need to ensure that the root is set before we call the CGI constructor.
*/
CGI::CGI(char ** cgi_env, std::string & executable, std::string & root)
{
	int			i = 0;

	while (cgi_env[i])
	{
		this->cgi_envp.push_back(cgi_env[i]);
		++i;
	}
	this->set_root(root);
	if (executable == "python3")
		this->cgi_path = this->get_root() + "/cgi-bin/py-cgi";
	else
		this->cgi_path = this->get_root() + "/cgi-bin/py-cgi";
	this->insert_arg(this->cgi_path);
}

CGI::~CGI()
{
	for (unsigned int i = 0; i < this->cgi_args.size(); ++i)
		delete this->cgi_args[i];
	for (unsigned int i = 0; i < this->cgi_envp.size(); ++i)
		delete this->cgi_envp[i];
}

char ** CGI::get_envp()
{
	int size = this->cgi_envp.size();
	char ** envp = new char *[size + 1];
	for (int i = 0; i < size; ++i)
	{
		// std::cerr << "The envp of " << i << " is " << this->cgi_envp[i] << std::endl;
		envp[i] = this->cgi_envp[i];
	}
	envp[size] = NULL;
	return envp;
}

std::string	CGI::get_cgi_path()
{
	return this->cgi_path;
}

char ** CGI::get_cgi_args()
{
	int size = this->cgi_args.size();
	char ** args = new char *[size + 1];
	for (int i = 0; i < size; ++i)
		args[i] = this->cgi_args[i];
	args[size] = NULL;
	return args;
}

std::string & CGI::get_root()
{
	return this->root;
}

void	CGI::set_root(std::string &root)
{
	this->root = root;
}

void	CGI::insert_arg(std::string & arg)
{
	this->cgi_args.push_back(strdup(arg.c_str()));
}
