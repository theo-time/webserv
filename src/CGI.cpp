#include "CGI.hpp"
#include <string>
#include <iostream>

std::string CGI::getContentType(Request & req) {

    std::cout << "HEADER" << req.requestString2 << std::endl;

    std::string boundaryPrefix = "Content-Type: ";
    size_t startPos = req.requestString2.find(boundaryPrefix);
    if (startPos == std::string::npos) {
        // "boundary" identifier not found
        return "";
    }

    startPos += boundaryPrefix.length();
    size_t endPos = req.requestString2.find_first_of("\r\n", startPos);

    if (endPos == std::string::npos) {
        // Line break not found
        return "";
    }

    std::string boundaryValue = req.requestString2.substr(startPos, endPos - startPos);
    return boundaryValue;

}

std::string getContentLength(Request & req) {

    std::string boundaryPrefix = "Content-Length: ";
    size_t startPos = req.requestString2.find(boundaryPrefix);
    if (startPos == std::string::npos) {
        // "boundary" identifier not found
        return "";
    }

    startPos += boundaryPrefix.length();
    size_t endPos = req.requestString2.find_first_of("\r\n", startPos);

    if (endPos == std::string::npos) {
        // Line break not found
        return "";
    }

    std::string boundaryValue = req.requestString2.substr(startPos, endPos - startPos);
    return boundaryValue;

}

CGI::CGI(Request & req)
{	
	std::string tmpBuf;
	
    //std::cout << "GET CONTENT" << getContentType(req) << std::endl;

	//_realUri = std::string(pwd) + str;
    _realUri = req.getPath();
	_exec = std::string("/usr/bin/python3");

    // TEST FOR req = GET
    int _req_meth = req.getMethodCode();

    // NEED QUERY STRING

    std::string _req_body = req.getBody();

	// ** set environment variable for the CGI **
	// GET : QUERY_STRING + PATH_INFO 
	// POST : PATH_INFO + CONTENT_length 
	if ((_envvar = new char*[7]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");
		
	int i = 0;
	//_envvar[i++] = strdup(("PATH_INFO=" + _realUri).c_str());
	_envvar[i++] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	_envvar[i++] = strdup("REDIRECT_STATUS=200");

	if (_req_meth == GET){



	}
    else if (_req_meth == POST){

        /*
        if (!_exec.compare("php-cgi"))
            _envvar[i++] = strdup("REQUEST_METHOD=GET");*/
        _envvar[i++] = strdup("REQUEST_METHOD=POST");
		_envvar[i++] = strdup(("CONTENT_TYPE=" + getContentType(req)).c_str());
        std::cout << "GET CONTENT" << _envvar[i - 1] << std::endl;
        _envvar[i++] = strdup("PATH_INFO=/data/testCGI/upload.py");
        std::stringstream intToString;
		intToString << _req_body.size();
        std::string tmpBuf = "CONTENT_LENGTH=" + getContentLength(req);
        _envvar[i++] = strdup(tmpBuf.c_str());
        std::cout << "CONTENT LENGTH" << _envvar[i - 1] << std::endl;
        _envvar[i++] = strdup("SCRIPT_FILENAME=data/testCGI/upload.py");
	    _envvar[i++] = strdup("SERVER_PROTOCOL=HTTP/1.1");
    }
	else {
        
        /*
        if (!_exec.compare("php-cgi"))
            _envvar[i++] = strdup("REQUEST_METHOD=POST");*/
		
	}
	
	_envvar[i++] = NULL;
    if ((_args = new char*[3]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");

	_args[0] = strdup(_exec.c_str());
	_args[1] = strdup("data/testCGI/upload.py");
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

        execve(_args[0], _args, _envvar);
        std::cout << "ALIVE 1" << std::endl;
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

        std::cout << "ALIVE 2" << std::endl;

        _response.setStatusCode("200");
        _response.setStatusText("OK");
        _response.setContentType("text/html");
        _response.setProtocol("HTTP/1.1");
        _response.setBody(outputCGI);
        _response.buildHeader();
        _response.buildResponse();
    }
}

Response CGI::getResponseCGI() {
    return _response;
}
