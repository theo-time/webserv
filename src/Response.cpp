
#include "Response.hpp"
#include "Request.hpp"

#include <string> 

Response::Response()
{

}

Response::~Response()
{
    std::cout << "Response destroyed" << std::endl;
}

/* Getters & Setters */

std::string Response::getResponse(){return response;}
std::string Response::getHeader(){return header;}
std::string Response::getBody(){return body;}
std::string Response::getProtocol(){return protocol;}
std::string Response::getStatusCode(){return statusCode;}
std::string Response::getStatusText(){return statusText;}
std::string Response::getContentType(){return contentType;}
std::string Response::getFilename(){return filename;}
std::string Response::getExtension(){return extension;}
std::string Response::getContentDisposition(){return contentDisposition;}

void Response::setProtocol(std::string protocol){this->protocol = protocol;}
void Response::setStatusCode(std::string statusCode){this->statusCode = statusCode;}
void Response::setStatusText(std::string statusText){this->statusText = statusText;}
void Response::setContentType(std::string contentType){this->contentType = contentType;}
void Response::setFilename(std::string filename){this->filename = filename;}
void Response::setExtension(std::string extension){this->extension = extension;}
void Response::setContentDisposition(std::string contentDisposition){this->contentDisposition = contentDisposition;}
void Response::setContentLength(std::string contentLength){this->contentLength = contentLength;}
void Response::setBody(std::string content){this->body = content;}
void Response::setRequest(Request *request){this->request = request;}

/* Methods */

void Response::buildHeader()
{
    std::stringstream ss;
    ss << body.length();
    std::string contentLength;
    ss >> contentLength;

    header = protocol + " " + statusCode + " " + statusText + "\r\nContent-Type: " + contentType + "\r\nContent-Disposition: " + contentDisposition + "; filename=\"" + filename + "\"\r\nContent-Length: " + contentLength + "\r\n\r\n";
}

void Response::buildResponse()
{
    response = header + body;
    std::cout << "----- Response : ----" << std::endl << response << std::endl;
}

void Response::send(void)
{
    buildHeader();
    buildResponse();
    request->ready2send = true;
}

void Response::send(const std::string& path)
{
    std::ifstream       file(path.c_str());
    // check file status
    if (!file.good())
    {
        sendError(500, ": Error when opening file " + path);
        return;
    }

    std::string         fileContent;
    std::stringstream   buffer;
    buffer << file.rdbuf();
    fileContent = buffer.str();
    file.close();
    
    setBody(fileContent);   
    send();
}

void Response::sendError(int statusCode, std::string error_msg)
{
    std::cout << statusCode << " " << error_msg << std::endl;
    setProtocol("HTTP/1.1");
    setContentType("text/html");
    setExtension("html");
    setContentDisposition("inline");

    std::stringstream ss;
    ss << statusCode;
    std::string errorPagePath = "./data/default/" + ss.str() + ".html";
    
    // check if custom error page exists
    if (request && request->getConfig()
        && request->getConfig()->getErrorPages().count(statusCode)
        && fileExists(request->getConfig()->getErrorPages()[statusCode]))
    {
        errorPagePath = request->getConfig()->getErrorPages()[statusCode];
    }

    switch(statusCode)
    {
        case 301:
            setStatusCode("301");
            setStatusText("Moved Permanently");
            setFilename("301.html");
            send(errorPagePath);
            break;
        case 400:
            setStatusCode("400");
            setStatusText("Bad Request");
            setFilename("400.html");
            send(errorPagePath);
            break;
        case 403:
            setStatusCode("403");
            setStatusText("Forbidden");
            setFilename("403.html");
            send(errorPagePath);
            break;
        case 404: 
            setStatusCode("404");
            setStatusText("Not Found");
            setFilename("404.html");
            send(errorPagePath);
            break;
        case 405:
            setStatusCode("405");
            setStatusText("Method Not Allowed");
            setFilename("405.html");
            send(errorPagePath);
            break;
        case 408:
            setStatusCode("408");
            setStatusText("Request Timeout");
            setFilename("408.html");
            send(errorPagePath);
            break;
        case 413:
            setStatusCode("413");
            setStatusText("Content Too Large");
            setFilename("413.html");
            send(errorPagePath);
            break;
        case 414:
            setStatusCode("414");
            setStatusText("URI Too Long");
            setFilename("414.html");
            send(errorPagePath);
            break;
        case 431:
            setStatusCode("431");
            setStatusText("Request Header Fields Too Large");
            setFilename("431.html");
            send(errorPagePath);
            break;
        case 500:
            setStatusCode("500");
            setStatusText("Internal Server Error");
            setFilename("500.html");
            send(errorPagePath);
            break;
        case 501:
            setStatusCode("501");
            setStatusText("Method Not Implemented");
            setFilename("501.html");
            send(errorPagePath);
            break;
        case 505:
            setStatusCode("505");
            setStatusText("HTTP Version Not Supported");
            setFilename("505.html");
            send(errorPagePath);
            break;
        default: 
            std::cout << "ERROR: unknown status code, sent status 500" << std::endl;
            setStatusCode("500");
            setStatusText("Internal Server Error");
            setFilename("500.html");
            send("./data/default/500.html");
    }
}

void Response::clear(void)
{
    protocol.clear();
    statusCode.clear();
    statusText.clear();
    contentType.clear();
    contentLength.clear();
    filename.clear();
    extension.clear();
    contentDisposition.clear();
    header.clear();
    body.clear();
    response.clear();
}
