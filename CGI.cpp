#include "CGI.hpp"

std::string getFileExtension(const std::string& url)
{
    // Find the position of the last dot in the URL
    size_t dotPos = url.rfind('.');
    if (dotPos == std::string::npos)
    {
        // No dot found, return empty string or handle error
        return "";
    }

    // Extract the substring starting from the dot position
    std::string extension = url.substr(dotPos + 1);

    return extension;
}

CGI::CGI(Request *req, const std::string &realUri, const std::string& exec)
{	
	std::string tmpBuf;
	
	char* pwd;
	if (!(pwd = getcwd(NULL, 0)))
		throw std::runtime_error("Error in getcwd command in cgi constructor\n");

	_realUri = pwd + ("/" + realUri);
	_exec = _realUri;

    // TEST FOR req = GET
    int _req_meth = GET;

    // NEED QUERY STRING
    std::string _query_str = "";
    std::string _req_body = "";

	// ** set environment variable for the CGI **
	// GET : QUERY_STRING + PATH_INFO 
	// POST : PATH_INFO + CONTENT_length 
	if ((_envvar = new char*[7]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");
		
	int i = 0;
	_envvar[i++] = strdup(("PATH_INFO=" + _realUri).c_str());
	_envvar[i++] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	_envvar[i++] = strdup("REDIRECT_STATUS=200");
	
	if (_req_meth == GET){

        /*
        if (!_exec.compare("php-cgi"))
            _envvar[i++] = strdup("REQUEST_METHOD=GET");*/
		
		tmpBuf = "QUERY_STRING=" + _query_str;
		_envvar[i++] = strdup(tmpBuf.c_str());

	}
	else {
        
        /*
        if (!_exec.compare("php-cgi"))
            _envvar[i++] = strdup("REQUEST_METHOD=POST");*/	
		
		std::string contentLength = std::to_string(_req_body.size());
        std::string tmpBuf = "CONTENT_LENGTH=" + contentLength;
        _envvar[i++] = strdup(tmpBuf.c_str());
	}
	
	_envvar[i++] = NULL;

	_args[0] = strdup(_exec.c_str());
	_args[1] = strdup(_realUri.c_str());
	_args[2] = NULL;
	
}

CGI::~CGI()
{
	int i = -1;
	while (_envvar[++i]){
		free(_envvar[i]); _envvar[i] = NULL;}
	delete[] _envvar;
	
	i = 0;
	while (_args[i++]){
		free(_args[i]); _args[i] = NULL;}
	delete[] _args;
}

void CGI::executeCGI()
{
    extension = getFileExtension(path);
    const char* scriptPath = "script.py";
    const char* argv[] = { "python", scriptPath, nullptr };
    const char* envp[] = { nullptr };


    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe failed");
    }

    pid_t pid = fork(); // Fork the process

    if (pid == -1)
    {
        perror("fork failed");
    }
    else if (pid == 0)  // Child process
    {
        close(pipefd[0]); // Close the read end of the pipe

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execve("/usr/bin/python3", const_cast<char**>(argv), const_cast<char**>(envp));
        perror("execve failed");
    }
    else {  // Parent process
        close(pipefd[1]); // Close the write end of the pipe

        // Read the output from the child process
        char buffer[4096];
        ssize_t bytesRead;

        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            outputCGI.append(buffer, bytesRead);
        }

        close(pipefd[0]); // Close the read end of the pipe

        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish

        if (WIFEXITED(status) == 0)
        {
            std::cout << "Script output:\n" << outputCGI << std::endl;
        }
        else
        {
            std::cout << "Child process terminated abnormally" << std::endl;
        }
    }
}

