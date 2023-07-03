#include "Request.hpp"
#include "CGI.hpp"

#include <fstream>


Request::Request(int clientSocket, int serverSocket) : clientSocket(clientSocket), serverSocket(serverSocket) {
    std::cout << "Request created" << std::endl;
}


void Request::parseRequest(){
    std::cout << "--------- REQUEST TO PARSE -------" << std::endl;
    std::cout << requestString << std::endl;
    requestString2 = requestString;
    std::cout << "--------- ++++++++++++++++ -------" << std::endl;
    // Parse first line
    std::string firstLine = requestString.substr(0, requestString.find("\r\n"));
    requestString.erase(0, requestString.find("\r\n") + 2);
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    int i = 0;
    while ((pos = firstLine.find(delimiter))) {
        token = firstLine.substr(0, pos);
        if (i == 0) {
            method = token;
        } else if (i == 1) {
            path = token;
        } else if (i == 2) {
            protocol = firstLine;
            break;
        }
        firstLine.erase(0, pos + delimiter.length());
        i++;
    }

    // verify method
    if(method == "GET") {
        methodCode = 1;
    } else if (method == "POST") {
        methodCode = 2;
        retrieveHeaderAndBody(requestString2);
    } else if (method == "DELETE") {
        methodCode = 3;
    } else {
        methodCode = 0;
    }

    // verify path
    if (path == "/") {
        path = "/index.html";
    }

    // Parse headers 
    delimiter = "\r\n";
    pos = 0;
    i = 0;
    while ((pos = requestString.find(delimiter)) != std::string::npos) {
        token = requestString.substr(0, pos);
        requestString.erase(0, pos + delimiter.length());
        std::string delimiter2 = ": ";
        size_t pos2 = 0;
        std::string token2;
        int j = 0;
        while ((pos2 = token.find(delimiter2)) != std::string::npos) {
            token2 = token.substr(0, pos2);
            token.erase(0, pos2 + delimiter2.length());
            if (j == 0) {
                headers[token2] = token;
            }
            j++;
        }
        i++;
    }


    // path = root + path;
    // std::cout << " - PARSING COMPLETED - " << std::endl;
    // std::cout << "Method: " << method << std::endl;
    // std::cout << "Path: " << path << std::endl;
    // std::cout << "Protocol: " << protocol << std::endl;
    // std::cout << "Headers: " << std::endl;
    // for (std::map<std::string, std::string>::iterator it=headers.begin(); it!=headers.end(); ++it)
    //     std::cout << it->first << " => " << it->second << '\n';
}

void Request::retrieveHeaderAndBody(const std::string& input) {
    std::string boundaryPrefix = "boundary=";
    size_t startPos = input.find(boundaryPrefix);
    if (startPos == std::string::npos) {
        // "boundary" identifier not found
        return;
    }

    startPos += boundaryPrefix.length();
    size_t endPos = input.find_first_of("\r\n", startPos);

    if (endPos == std::string::npos) {
        // Line break not found
        return;
    }

    std::string boundaryValue = input.substr(startPos, endPos - startPos);
    boundaryValue = "--" + boundaryValue;

    std::cout << "boundaryValue :" << boundaryValue << std::endl;

    // Find the start and end positions of the body
    std::string bodyStart = boundaryValue;
    size_t bodyStartPos = input.find(bodyStart);
    if (bodyStartPos == std::string::npos) {
        std::cout << "BODY START NOT FOUND" << std::endl;
        return;
    }

    bodyStartPos += bodyStart.length();
    size_t bodyEndPos = input.find(boundaryValue + "--", bodyStartPos);
    if (bodyEndPos == std::string::npos) {
        std::cout << "BODY END NOT FOUND" << std::endl;
        return;
    }

    // Extract the header and body
    header = input.substr(0, bodyStartPos - bodyStart.length());
    body = input.substr(bodyStartPos, bodyEndPos - bodyStartPos);
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

    //std::cout << getFileExtension(path) << std::endl;

    if((getFileExtension(path) == "py") && (methodCode == GET || methodCode == POST))
    {
        if (methodCode == POST){
            retrieveHeaderAndBody(requestString2);
        }

        CGI cgi(path);
        std::cout << "PATH :" << path << std::endl;

        std::cout << "BODY :" << body << std::endl;

        cgi.executeCGI();
        _response = cgi.getResponseCGI();
        WebServ::addCGIResponseToQueue(this);
        return;
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

int Request::getServerSocket() {
    return serverSocket;
}

std::string Request::getResponseString() {
    return _response.getResponse();
}

std::string Request::getHeader(std::string key) {
    return headers[key];
}

VirtualServer* Request::getConfig() {
    return _config;
}

// SETTERS 

void Request::setConfig(VirtualServer* config) {
    _config = config;
}

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