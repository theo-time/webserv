#include "Request.hpp"
#include <fstream>

Request::Request(int clientSocket, std::string request): clientSocket(clientSocket), request(request) {
    std::cout << "Request created" << std::endl;

    // Conf file variables
    std::string root = "/mnt/nfs/homes/teliet/dev/web_serv";

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

    // verify method
    if(method == "GET") {
        type = 1;
    } else if (method == "POST") {
        type = 2;
    } else if (method == "DELETE") {
        type = 3;
    } else {
        type = 0;
    }

    // verify path
    if (path == "/") {
        path = "/index.html";
    }
    // path = root + path;

    std::cout << "Method: " << method << std::endl;

    std::cout << "Path: " << path << std::endl;

    std::cout << "Protocol: " << protocol << std::endl;


}

Request::~Request() {
    std::cout << "Request destroyed" << std::endl;
}

std::string getFileExtension(std::string filename)
{
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    return extension;
}

void Request::handleRequest() 
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

    // read file  
    // text file
    // std::string line;
    // while (std::getline(file, line)) {
    //     fileContent += line;
    // }
    // file.close();

    // binary file
    std::stringstream buffer;
    buffer << file.rdbuf();
    fileContent = buffer.str();



    // Build response
    std::string status = "200";
    std::string statusText = "OK";
    std::string contentType = "text/plain";
    std::string filename = path.substr(path.find_last_of("/") + 1);

    std::stringstream ss;
    ss << fileContent.length();
    std::string contentLength;
    ss >> contentLength;

    std::string extension = getFileExtension(path);
    std::string contentDisposition = "inline";
    // if extension is not displayable, download it
    if(extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "gif" || extension == "bmp" || extension == "ico") {
        contentType = "image/" + extension;
        contentDisposition = "attachment";
    } else if (extension == "pdf") {
        contentType = "application/pdf";
        contentDisposition = "attachment";
    } else if (extension == "css") {
        contentType = "text/css";
    } else if (extension == "js") {
        contentType = "application/javascript";
    } else if (extension == "txt") {
        contentType = "text/plain";
    } else if (extension == "html") {
        contentType = "text/html";
    } else {
        contentType = "text/plain";
    }


    response = "HTTP/1.1 " + status + " " + statusText + "\r\nContent-Type: " + contentType + "\r\nContent-Disposition: " + contentDisposition + "; filename=\"" + filename + "\"\r\nContent-Length: " + contentLength + "\r\n\r\n" + fileContent;
    // response = "HTTP/1.1 " + status + " " + statusText + "\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent;
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

