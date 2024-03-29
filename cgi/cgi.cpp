#include "cgi.hpp"
#include "../WebServ.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void	freeCharArray(char **array, size_t size)
{
	for (size_t i = 0; array && i < size; ++i) {
		if (array[i])
			delete array[i];
	}
	if (array)
		delete[] array;
	array = NULL;
}

void	CGI::execute_cgi(HttpRequest &request, CGI *cgi, std::string filepath,
	int &status_code, std::vector<char>& output)
{
	int			fd[2];
	pid_t		pid;

	cgi->insert_arg(filepath);
	cgi->insert_arg(request.method);
	std::string	vecToStr(request.body.begin(), request.body.end());
	cgi->insert_arg(vecToStr);
	if (pipe(fd) == -1)
		throw std::exception();
	fcntl(fd[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
    fcntl(fd[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC);

	pid = fork();
	if (pid == -1)
		throw std::exception();
	char **args = NULL;
	char **envp = NULL;
	if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		args = cgi->get_cgi_args();
		envp = cgi->get_envp();
		if (execve(cgi->get_cgi_path().c_str(), args, envp) == -1)
		{
			freeCharArray(args, cgi_args.size());
			freeCharArray(envp, cgi_envp.size());
			std::cerr << RED << "Error: The CGI code crashed" << RESETCLR << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else
	{
    	close(fd[1]);
		int status;

		fd_set set;
		FD_ZERO(&set);
		FD_SET(fd[0], &set);

		struct timeval timeout;
		timeout.tv_sec = CLIENT_TIMEOUT_CGI;
		timeout.tv_usec = 0;

		int result = select(fd[0] + 1, &set, NULL, NULL, &timeout);
		if (result > 0)
		{
			waitpid(pid, &status, 0);
			kill(pid, SIGKILL);
			if (WIFEXITED(status))
			{
				if (WEXITSTATUS(status) != 0)
				{
					status_code = 500;
					output.clear();
				}
				else
				{

					status_code = 200;
					char buffer[1024];
					int	bytes_read;
					std::string	toAppend;
					do
					{
						std::memset(buffer, 0, 1024);
						toAppend.clear();
						bytes_read = read(fd[0], buffer, 1023);
						if (bytes_read == -1 || bytes_read == 0)
							break;
						toAppend = buffer;
						output.insert(output.end(), toAppend.begin(), toAppend.end());
					}
					while (bytes_read > 0);
				}
			}
		}
		else if (result == 0)
        {
            kill(pid, SIGKILL);
            sleep(1);
            status_code = 408;
            output.clear();
        }
		close(fd[0]);
	}
	freeCharArray(args, cgi_args.size());
	freeCharArray(envp, cgi_envp.size());
}

/**
 * Since we need the root, we need to get the root from the config file
 * The root will be dependant on the request that comes in to the code
 * We need to ensure that the root is set before we call the CGI constructor.
*/
CGI::CGI(char ** cgi_env, std::string & executable)
{
	int			i = 0;

	while (cgi_env[i])
	{
		this->cgi_envp.push_back(cgi_env[i]);
		++i;
	}
	this->cgi_path = executable;
	this->insert_arg(this->cgi_path);
}

CGI::~CGI()
{
	for (unsigned int i = 0; i < this->cgi_args.size(); ++i)
		delete this->cgi_args[i];
}

char ** CGI::get_envp()
{
	int size = this->cgi_envp.size();
	char ** envp = new char *[size + 1];
	for (int i = 0; i < size; ++i)
		envp[i] = this->cgi_envp[i];
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
