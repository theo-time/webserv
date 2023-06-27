#include "Request.hpp"

#include <fstream>


Request::Request(int clientSocket): clientSocket(clientSocket) {
    std::cout << "Request created" << std::endl;
}


void Request::parseRequest(){
    std::cout << "Request created" << std::endl;

    // Parse request 
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    int i = 0;
    while ((pos = requestString.find(delimiter)) != std::string::npos) {
        token = requestString.substr(0, pos);
        if (i == 0) {
            method = token;
        } else if (i == 1) {
            path = token;
        } else if (i == 2) {
            protocol = token;
        }
        requestString.erase(0, pos + delimiter.length());
        i++;
    }

    // verify method
    if(method == "GET") {
        methodCode = 1;
    } else if (method == "POST") {
        methodCode = 2;
    } else if (method == "DELETE") {
        methodCode = 3;
    } else {
        methodCode = 0;
    }

    // verify path
    if (path == "/") {
        path = "/index.html";
    }

    // path = root + path;
    std::cout << " - PARSING COMPLETED -" << method << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Protocol: " << protocol << std::endl;
}

std::string getFileExtension(std::string filename)
{
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    return extension;
}


void Request::buildResponse()
{

    std::string extension = getFileExtension(path);
    std::string filename = path.substr(path.find_last_of("/") + 1);
    std::string contentType = "text/plain";
    std::string contentDisposition = "inline";

    // Build response
    _response.setStatusCode("200");
    _response.setStatusText("OK"); 
    _response.setProtocol("HTTP/1.1");
    _response.setFilename(filename);
    _response.setExtension(extension);

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

    _response.setContentType(contentType);
    _response.setContentDisposition(contentDisposition);
    _response.buildHeader();
    _response.buildResponse();
}


Request::~Request() {
    std::cout << "Request destroyed" << std::endl;
}

void Request::get() 
{
    std::ifstream my_file(path.c_str());
    if (!my_file.good())
    {
        std::cout << "File not found" << std::endl;
        _response.setStatusCode("404");
        _response.setStatusText("Not Found");
        _response.setContentType("text/html");
        WebServ::getRessource("./data/default/404.html", *this);
    }
    else 
    {
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        std::cout << "Requesting ressource at path : " << path << std::endl;
        WebServ::getRessource(path, *this);
    }
}

void Request::post() 
{
    std::cout << "POST" << std::endl;
}

void Request::mdelete() 
{
    std::cout << "DELETE" << std::endl;
    std::ifstream my_file(path.c_str());

    if (!my_file.good())
    {
        std::cout << "File not found" << std::endl;
        _response.setStatusCode("404");
        _response.setStatusText("Not Found");
        _response.setContentType("text/html");
        WebServ::getRessource("./data/default/404.html", *this);
    }
    else 
    {
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        std::cout << "Requesting ressource at path : " << path << std::endl;
        if(!remove(path.c_str()))
        {
            std::cout << "File deleted" << std::endl;
            // TODO : send 200 response without body
        }
        else
        {
            std::cout << "Error deleting file" << std::endl;
            _response.setStatusCode("500");
            _response.setStatusText("Internal Server Error");
            _response.setContentType("text/html");
            WebServ::getRessource("./data/default/500.html", *this);
        }

    }
}

void Request::handleRequest() 
{

    // Conf file variables
    std::string root = "/mnt/nfs/homes/teliet/dev/web_serv";

    std::cout << "Handle request" << std::endl;
    std::string fileContent;

    if(cgi_mode)
    {
        
    }

    // Read file (add "." before path to read from current directory)
    path = "." + path; // TODO : add root to path

    // Method routing
    if (methodCode == GET)
        this->get();
    if(methodCode == POST)
        this->post();
    if(methodCode == DELETE)
        this->mdelete();
    if(methodCode == UNKNOWN)
    {
        std::cout << "Unknown method" << std::endl;
        _response.setStatusCode("501");
        _response.setStatusText("Not Implemented");
        _response.setContentType("text/html");
        WebServ::getRessource("./data/default/501.html", *this);
    }
}

// GETTERS

std::string Request::getPath() {
    return path;
}

std::string Request::getProtocol() {
    return protocol;
}

std::string Request::getMethod() {
    return method;
}

std::string Request::getRequestString() {
    return requestString;
}

int Request::getClientSocket() {
    return clientSocket;
}

std::string Request::getResponseString() {
    return _response.getResponse();
}

// SETTERS 

void Request::setFileContent(std::string &fc)
{
    std::cout << "Setting file content" << std::endl;
    std::cout << fc << std::endl;
    fileContent = fc;
    _response.setBody(fileContent);
}

void Request::setResponseString(std::string &rs)
{
    responseString = rs;
}

void Request::setRequestString(std::string &rs)
{
    requestString = rs;
}