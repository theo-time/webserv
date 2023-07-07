
#include "Response.hpp"

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
    // std::cout << "----- Response : ----" << std::endl << response << std::endl;
}

void Response::sendError(int statusCode, std::string statusText)
{
    (void)statusText;
    switch(statusCode)
    {
        case 404: 
            setStatusCode("404");
            setStatusText("Not Found");
            setContentType("text/html");
            setFilename("404.html");
            setExtension("html");
            setContentDisposition("inline");
            buildHeader();
            WebServ::getRessource("./data/default/404.html", *request);
            break;
        case 403:
            setStatusCode("403");
            setStatusText("Forbidden");
            setContentType("text/html");
            setFilename("403.html");
            setExtension("html");
            setContentDisposition("inline");
            buildHeader();
            buildResponse();
            WebServ::getRessource("./data/default/403.html", *request);
            break;
        case 400:
            setStatusCode("400");
            setStatusText("Bad Request");
            setContentType("text/html");
            setFilename("400.html");
            setExtension("html");
            setContentDisposition("inline");
            buildHeader();
            WebServ::getRessource("./data/default/400.html", *request);
            break;
        case 405:
            setStatusCode("405");
            setStatusText("Method Not Allowed");
            setContentType("text/html");
            setFilename("405.html");
            setExtension("html");
            setContentDisposition("inline");
            buildHeader();
            WebServ::getRessource("./data/default/405.html", *request);
            break;
        case 500:
            setStatusCode("500");
            setStatusText("Internal Server Error");
            setContentType("text/html");
            setFilename("500.html");
            setExtension("html");
            setContentDisposition("inline");
            buildHeader();
            WebServ::getRessource("./data/default/500.html", *request);
            break;
        default:
            std::cout << "Fatal Error : Unknown status code" << std::endl;
            exit(1);
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
