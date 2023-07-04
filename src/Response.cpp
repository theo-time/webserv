
#include "Response.hpp"

Response::Response()
{
    std::cout << "Response created" << std::endl;
}

Response::~Response()
{
    std::cout << "Response destroyed" << std::endl;
}

/* Getters & Setters */

std::string Response::getResponse(){return response;}

void Response::setProtocol(std::string protocol){this->protocol = protocol;}
void Response::setStatusCode(std::string statusCode){this->statusCode = statusCode;}
void Response::setStatusText(std::string statusText){this->statusText = statusText;}
void Response::setContentType(std::string contentType){this->contentType = contentType;}
void Response::setFilename(std::string filename){this->filename = filename;}
void Response::setExtension(std::string extension){this->extension = extension;}
void Response::setContentDisposition(std::string contentDisposition){this->contentDisposition = contentDisposition;}
void Response::setContentLength(std::string contentLength){this->contentLength = contentLength;}
void Response::setBody(std::string content){this->body = content;}


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
    // std::cout << "----- Response : ----" << std::endl << response << std::endl;
}