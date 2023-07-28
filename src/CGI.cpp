#include "CGI.hpp"
#include <unistd.h>


CGI::CGI(Request & req) : _req(req)// Initialize all environment variable for CGI
{
    std::string cur_wd = get_current_dir_name();
    std::cout << "CURRENT DIRECTORY : " << get_current_dir_name() << std::endl;
	if ((_envvar = new char*[10]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");
	int i = 0;
    _envvar[i++] = strdup("SERVER_PROTOCOL=HTTP/1.1");
    _envvar[i++] = strdup(("PATH_INFO="+ req.getPath()).c_str());
    _envvar[i++] = strdup(("SCRIPT_FILENAME=" + cur_wd + req.getPath().substr(1)).c_str());
    _envvar[i++] = strdup(("SCRIPT_NAME=" + req.getPath()).c_str());
    _envvar[i++] = strdup(("REDIRECT_STATUS=200"));
    _envvar[i++] = strdup(("DOCUMENT_ROOT=" + cur_wd).c_str());
    

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

bool CGI::executeCGI(Request & req)
{
    if (pipe(pipe_in) < 0){
        perror("pipe failed");
        close (pipe_in[1]);
        close (pipe_in[0]);
        return false;
    }
    if (pipe(pipe_out) < 0) {
        perror("pipe failed");
        close (pipe_in[1]);
        close (pipe_in[0]);
        return false;
    }
    std::cout << "BODY FED : " << req.getBody().c_str() << std::endl;
    std::cout << _args[1] << std::endl;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        close (pipe_in[1]);
        close (pipe_in[0]);
        return false;
    }

    else if (pid == 0)  // Child process
    {
		dup2(pipe_out[1], STDOUT_FILENO);
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        write(pipe_in[1], req.getBody().c_str(), _req_body.length());
        //std::cout << "WRITE BYTES : " << tmp << std::endl;

        execve(_args[0], _args, _envvar);
        //exit(1);
        //perror("execve failed");
        exit(0);
    }
    else {  // Parent process
        char buffer[4096];
        ssize_t bytesRead;
        int status;

        close(pipe_in[1]);
        close(pipe_in[0]);

        waitpid(pid, &status, 0); // Wait for the child process to finish
        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus == 0) {
                std::cout << "CGI execution was successful." << std::endl;
            } else {
                std::cout << "CGI execution failed with exit status: " << exitStatus << std::endl;
                close(pipe_out[1]);
                while ((bytesRead = read(pipe_out[0], buffer, 4096)) > 0) // Read the output from the child process
                    outputCGI.append(buffer, bytesRead);
                std::cout << "---- OUTPUTCGI ----"<< std::endl;
                std::cout << outputCGI << std::endl;
                std::cout << "---- END OUTPUTCGI ----"<< std::endl;
                return false;
            }
        } else {
            std::cout << "CGI execution failed due to an unknown reason." << std::endl;
            return false;
        }
        close(pipe_out[1]);
        while ((bytesRead = read(pipe_out[0], buffer, 4096)) > 0) // Read the output from the child process
            outputCGI.append(buffer, bytesRead);
        std::cout << "---- OUTPUTCGI ----"<< std::endl;
        std::cout << outputCGI << std::endl;
        std::cout << "---- END OUTPUTCGI ----"<< std::endl;
        close(pipe_out[0]); // Close the read end of the pipe
        return true;
    }
}