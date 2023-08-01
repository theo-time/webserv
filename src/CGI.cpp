#include "CGI.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <fcntl.h>

CGI::CGI(Request & req) : _req(req)// Initialize all environment variable for CGI
{
    char* cur_wd = get_current_dir_name();
    std::string current_dir = cur_wd;
	if ((_envvar = new char*[10]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");
	int i = 0;
    _envvar[i++] = strdup("SERVER_PROTOCOL=HTTP/1.1");
    _envvar[i++] = strdup(("PATH_INFO="+ req.getPath()).c_str());
    _envvar[i++] = strdup(("SCRIPT_FILENAME=" + current_dir + req.getPath().substr(1)).c_str());
    _envvar[i++] = strdup(("SCRIPT_NAME=" + req.getPath()).c_str());
    _envvar[i++] = strdup(("REDIRECT_STATUS=200"));
    _envvar[i++] = strdup(("DOCUMENT_ROOT=" + current_dir).c_str());
    

    //if (getContentInfo(req, "Content-Type: ") == "")

	if (req.getMethod() == "GET"){
        _envvar[i++] = strdup("REQUEST_METHOD=GET");
        _envvar[i++] = strdup(("QUERY_STRING=" + req.getQuery()).c_str());
        std::cout << "Query : " << _envvar[i - 1] << std::endl;
	}
    else if (req.getMethod() == "POST"){
        _envvar[i++] = strdup("REQUEST_METHOD=POST");
		_envvar[i++] = strdup(("CONTENT_TYPE=" + getContentInfo(req, "Content-Type: ")).c_str());
        _envvar[i++] = strdup(("CONTENT_LENGTH=" + getContentInfo(req, "Content-Length: ")).c_str());
        /*std::cout << "CONTENT TYPE: " << _envvar[i - 2] << std::endl;
        std::cout << "CONTENT LENGTH: " << _envvar[i - 1] << std::endl;*/
    }
	_envvar[i++] = NULL;

    if ((_args = new char*[3]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");

    _args[0] = strdup(req.executable_path.c_str());
	_args[1] = strdup(req.script_path.c_str());
    //_args[1] = NULL;
	_args[2] = NULL;

    free(cur_wd);
}

CGI::~CGI()
{
	int i = -1;
	while (_envvar[++i]){
		free(_envvar[i]); _envvar[i] = NULL;}
	delete[] _envvar;
	
	i = 0;
	while (_args[i]){
		free(_args[i]); _args[i] = NULL;
        i++;
    }
	delete[] _args;
}

volatile sig_atomic_t alarm_triggered = 0;

void handle_alarm(int) {
    alarm_triggered = 1;
}

bool CGI::executeCGI(Request & req)
{
    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0){
        perror("pipe failed");
        close (pipe_in[1]);
        close (pipe_in[0]);
        return false;
    }
    if (req.getMethod() == "POST") {
        WebServ::addFd2Select(pipe_in[1]);
        if(write(pipe_in[1], req.getBody().c_str(), req.getBody().length()) < 0)
            return false;
        WebServ::delFd2Select(pipe_in[1]);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        close (pipe_in[1]);
        close (pipe_in[0]);
        close (pipe_out[1]);
        close (pipe_out[0]);
        return false;
    }
    else if (pid == 0)  // Child process
    {
		dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[0]);
        close(pipe_out[1]);
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[0]);
        close(pipe_in[1]);
        /*std::cout << "" << std::endl;
        std::cout << "AAAAAAAAAAA" << std::endl;
        std::cout << "" << std::endl;*/
        execve(_args[0], _args, _envvar);
        //WebServ::stop();
        
        exit(EXIT_FAILURE);
    }
    else {  // Parent process
        signal(SIGALRM, handle_alarm);

        close(pipe_out[1]);
        close(pipe_in[1]);
        close(pipe_in[0]);

        int status = 0;

        ssize_t bytesRead;
        char buffer[10];

        alarm(5);

        //int err = 0;
        WebServ::addFd2Select(pipe_out[0]);
        // Wait for the child process to exit or data to be available in the pipe_out
        while (waitpid(pid, &status, WNOHANG) == 0) {
                //std::cout << "loop : " << i << std::endl;
                // Data available to read from the child process
                /*if (status != 0)
                    err = status;*/
                if (alarm_triggered == 1) {
                    kill(pid, SIGTERM);
                }
                bytesRead = read(pipe_out[0], buffer, sizeof(buffer));
                if (bytesRead > 0) {
                    outputCGI.append(buffer, bytesRead);
                }
                else if (bytesRead == 0) {
                    break;
                }
                else {
                    std::cout << "Child process error in read" << std::endl;
                    return false;
                }
        }
        WebServ::delFd2Select(pipe_out[0]);

        // After the loop, the child process has either exited or its output is fully read.
        close(pipe_out[0]);

        alarm(0);
        alarm_triggered = 0;

        outputCGI = removeContentTypeHeader(outputCGI);

        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus == 0) {
                std::cout << "CGI execution was successful." << std::endl;
                return true;
            } else {
                std::cout << "CGI execution failed with exit status: " << exitStatus << std::endl;
                return false;
            }
        } else {
            std::cout << "CGI execution failed due to an unknown reason." << std::endl;
            return false;
        }
    }
}