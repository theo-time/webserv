#include "CGI.hpp"

CGI::CGI(Request & req) // Initialize all environment variable for CGI
{
    /*std::cout << "PATHCGI: " << req.getPath() << std::endl;
    std::string _req_body = req.getBody();*/

	if ((_envvar = new char*[7]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");
		
	int i = 0;
	//_envvar[i++] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	_envvar[i++] = strdup("REDIRECT_STATUS=200");
    _envvar[i++] = strdup(("PATH_INFO=" + path).c_str());
    _envvar[i++] = strdup(("SCRIPT_FILENAME=" + path).c_str());
    _envvar[i++] = strdup(("SERVER_PROTOCOL=" + req.getProtocol()).c_str());
    //_envvar[i++] = strdup("QUERY_STRING=first_name=AA&last_name=AAA");

	if (req.getMethodCode() == GET){

        _envvar[i++] = strdup("REQUEST_METHOD=GET");

	}
    else if (req.getMethodCode() == POST){

        _envvar[i++] = strdup("REQUEST_METHOD=POST");
		_envvar[i++] = strdup(("CONTENT_TYPE=" + getContentInfo(req, "Content-Type: ")).c_str());
        _envvar[i++] = strdup(("CONTENT_LENGTH=" + getContentInfo(req, "Content-Length: ")).c_str());
        /*std::cout << "CONTENT TYPE: " << _envvar[i - 1] << std::endl;
        std::cout << "CONTENT LENGTH: " << _envvar[i - 1] << std::endl;*/
    }
	_envvar[i++] = NULL;


    if ((_args = new char*[3]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");

    std::cout << "PATH: " << req.getPath().c_str() << std::endl;
    std::cout << "PROTOCOL: " << req.getProtocol().c_str() << std::endl;

	_args[0] = strdup("/usr/bin/python3");
	_args[1] = strdup(req.getPath().c_str());
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

    int pipefd[2];
    if (pipe(pipefd) == -1)
        perror("pipe failed");

    pid_t pid = fork();
    if (pid == -1)
        perror("fork failed");
    else if (pid == 0)  // Child process
    {
        close(pipefd[0]); // Close the read end of the pipe
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
        close(pipefd[1]);

        //std::cout << "BEFORE EXECVE: " << std::endl;
        execve(_args[0], _args, _envvar);
        perror("execve failed");
    }
    else {  // Parent process
        char buffer[4096];
        ssize_t bytesRead;
        int status;

        //std::cout << "IN PARENT PROCESS " << std::endl;
        close(pipefd[1]); // Close the write end of the pipe
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) // Read the output from the child process
            outputCGI.append(buffer, bytesRead);

        //std::cout << "OUTPUTCGI: "<< outputCGI << std::endl;
        close(pipefd[0]); // Close the read end of the pipe
        waitpid(pid, &status, 0); // Wait for the child process to finish
        if (WIFEXITED(status))
			std::cout << "CGI execution was successful." << std::endl;
		else
			std::cout << "CGI execution failed." << std::endl;
    }
}

std::string CGI::getContentInfo(Request & req, std::string str) {

    std::string contentPrefix = str;
    size_t startPos = req.requestString2.find(contentPrefix);
    if (startPos == std::string::npos)
        return "";

    startPos += contentPrefix.length();
    size_t endPos = req.requestString2.find_first_of("\r\n", startPos);
    if (endPos == std::string::npos)
        return "";

    std::string contentValue = req.requestString2.substr(startPos, endPos - startPos);
    return contentValue;
}

Response CGI::getResponseCGI(Request & req) {
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        _response.setContentType("text/html");
        _response.setProtocol(req.getProtocol());
        _response.setBody(outputCGI);
        _response.buildHeader();
        _response.buildResponse();
        return _response;
}