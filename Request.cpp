#include "Request.hpp"
#include <fstream>

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


Request::Request(int clientSocket, std::string request): clientSocket(clientSocket), request(request) {
    std::cout << "Request created" << std::endl;

    // Parse request 
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    int i = 0;
    while ((pos = request.find(delimiter)) != std::string::npos) {
        token = request.substr(0, pos);
        if (i == 0) {
            method = token;
        } else if (i == 1) {
            path = token;
        } else if (i == 2) {
            protocol = token;
        }
        request.erase(0, pos + delimiter.length());
        i++;
    }

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

    // verify path
    if (path == "/") {
        path = "/index.html";
    }

    std::cout << "Method: " << method << std::endl;

    std::cout << "Path: " << path << std::endl;

    std::cout << "Protocol: " << protocol << std::endl;


}

Request::~Request() {
    std::cout << "Request destroyed" << std::endl;
}

void Request::prepareResponse() 
{
    std::cout << "Handle request" << std::endl;
    std::string fileContent;

    if (type != GET) 
        response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n\r\n405 Method Not Allowed";

    // Read file (add "." before path to read from current directory)
    path = "." + path;
    std::ifstream file(path.c_str());     

    // Check if file exists
    if (!file.good())
    {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        std::cout << "File not found" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        fileContent += line;
    }


    // Build response
    if (extension == "py") {
        response = "HTTP/1.1 200 OK\r\n" + outputCGI;
    }
    else {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + fileContent;
    }



}

// GETTERS

int Request::getType() {
    return type;
}

std::string Request::getPath() {
    return path;
}

std::string Request::getProtocol() {
    return protocol;
}

std::string Request::getMethod() {
    return method;
}

std::string Request::getRequest() {
    return request;
}

int Request::getClientSocket() {
    return clientSocket;
}

std::string Request::getResponse() {
    return response;
}

