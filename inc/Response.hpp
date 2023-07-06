#ifndef RESPONSE_HPP
#define RESPONSE_HPP 


#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

#include "WebServ.hpp"

class Response {
    private:
        std::string protocol;
        std::string statusCode;
        std::string statusText;
        std::string contentType;
        std::string contentLength;
        std::string filename;
        std::string extension;
        std::string contentDisposition;

        std::string header;
        std::string body;
        std::string response;

        Request *request;

    public:
        Response();
        ~Response();

        /* Getters & Setters */
        void setProtocol(std::string protocol);
        void setStatusCode(std::string statusCode);
        void setStatusText(std::string statusText);
        void setContentType(std::string contentType);
        void setFilename(std::string filename);
        void setExtension(std::string extension);
        void setContentDisposition(std::string contentDisposition);
        void setContentLength(std::string contentLength);
        void setBody(std::string content);
        void setRequest(Request *request);
        std::string getResponse();

        void buildHeader();
        void buildResponse();
        void sendError(int statusCode, std::string statusText);

};

#endif