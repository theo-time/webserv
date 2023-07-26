#include "CGI.hpp"

CGI::CGI(Request & req) // Initialize all environment variable for CGI
{
	if ((_envvar = new char*[8]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");
		
	int i = 0;
    _envvar[i++] = strdup("SERVER_PROTOCOL=HTTP/1.1");
    _envvar[i++] = strdup(("PATH_INFO=" + req.getPath()).c_str());
    //_envvar[i++] = strdup(("SCRIPT_NAME=" + req.getPath()).c_str());


    //if (getContentInfo(req, "Content-Type: ") == "")

	if (req.getMethod() == "GET"){
        //_req_body = "AAAAAA";
        _envvar[i++] = strdup("REQUEST_METHOD=GET");
        _envvar[i++] = strdup(("QUERY_STRING=" + req.getQuery()).c_str());
        std::cout << "Query : " << _envvar[i - 1] << std::endl;
	}
    else if (req.getMethod() == "POST"){
        
        //_req_body = "Hello worldaaa!";
        _envvar[i++] = strdup("REQUEST_METHOD=POST");
		_envvar[i++] = strdup(("CONTENT_TYPE=" + getContentInfo(req, "Content-Type: ")).c_str());
        _envvar[i++] = strdup(("CONTENT_LENGTH=" + getContentInfo(req, "Content-Length: ")).c_str());
        /*std::cout << "CONTENT TYPE: " << _envvar[i - 2] << std::endl;
        std::cout << "CONTENT LENGTH: " << _envvar[i - 1] << std::endl;*/
    }
	_envvar[i++] = NULL;

    if ((_args = new char*[3]) == NULL)
		throw std::runtime_error("Error on a cgi malloc\n");

    //std::cout << "SCRIPT_NAME: " << req.getPath().c_str() << std::endl;

    _args[0] = strdup(req.executable_path.c_str());
	_args[1] = strdup(req.script_path.c_str());
	_args[2] = NULL;
}

CGI::~CGI()
{
	int i = -1;
	while (_envvar[++i]){
		free(_envvar[i]); _envvar[i] = NULL;}
	delete[] _envvar;
	
	i = -1;
	while (_args[i++]){
		free(_args[i]); _args[i] = NULL;}
	delete[] _args;
}

void CGI::executeCGI(Request & req)
{
    if (pipe(pipe_in) < 0){
        perror("pipe failed");
        close (pipe_in[1]);
        close (pipe_in[0]);
        return;
    }
    if (pipe(pipe_out) < 0) {
        perror("pipe failed");
        close (pipe_in[1]);
        close (pipe_in[0]);
    }
    std::cout << "BODY FED : " << req.getBody().c_str() << std::endl;
    std::cout << _args[1] << std::endl;
    write(pipe_in[1], req.getBody().c_str(), _req_body.length());
    //std::cout << "WRITE BYTES : " << tmp << std::endl;
    pid_t pid = fork();
    if (pid == -1)
        perror("fork failed");
    else if (pid == 0)  // Child process
    {
        dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);

        execve(_args[0], _args, _envvar);
        //exit(1);
        perror("execve failed");
    }
    else {  // Parent process
        char buffer[4096];
        ssize_t bytesRead;
        int status;

        close(pipe_in[1]);
        close(pipe_in[0]);

        waitpid(pid, &status, 0); // Wait for the child process to finish
        if (WIFEXITED(status))
			std::cout << "CGI execution was successful." << std::endl;
		else
			std::cout << "CGI execution failed." << std::endl;

        close(pipe_out[1]);
        while ((bytesRead = read(pipe_out[0], buffer, 4096)) > 0) // Read the output from the child process
            outputCGI.append(buffer, bytesRead);
        std::cout << "OUTPUTCGI: "<< outputCGI << std::endl;
        std::cout << "END OUTPUTCGI: "<< std::endl;
        close(pipe_out[0]); // Close the read end of the pipe
    }
}

std::string CGI::getContentInfo(Request & req, std::string str) {

    std::string contentPrefix = str;
    size_t startPos = req.getRequestString().find(contentPrefix);
    if (startPos == std::string::npos)
        return "";

    startPos += contentPrefix.length();
    size_t endPos = req.getRequestString().find_first_of("\r\n", startPos);
    if (endPos == std::string::npos)
        return "";

    std::string contentValue = req.getRequestString().substr(startPos, endPos - startPos);
    return contentValue;
}

Response CGI::getResponseCGI() {
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        _response.setContentType("text/html");
        _response.setProtocol("HTTP/1.1");
        _response.setBody(outputCGI);
        _response.buildHeader();
        _response.buildResponse();
        return _response;
}