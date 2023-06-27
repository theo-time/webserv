#ifndef REQUEST_HPP
#define REQUEST_HPP 


#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>

# include "WebServ.hpp"
# include "VirtualServer.hpp"
# include "Response.hpp"

// TYPES 
#define GET 1
#define POST 2
#define DELETE 3
#define UNKNOWN 0

class Request {
    private:
        int clientSocket;
        std::string method;
        std::string path;
        std::string protocol;
        std::string requestString;
        std::string responseString;
        std::string fileContent;
        int methodCode;
        bool cgi_mode;

        VirtualServer       _server;
        Response            _response;  

    public:
        Request(int clientSocket);
        ~Request();

        int getType();
        std::string getPath();
        std::string getProtocol();
        std::string getMethod();
        std::string getRequestString();
        int         getClientSocket();
        std::string getResponseString();
        void setFileContent(std::string &fileContent);
        void setResponseString(std::string &response);
        void setRequestString(std::string &request);

        void get();
        void post();
        void mdelete();

        void parseRequest();
        void handleRequest();
        void buildResponse();
        bool fileExists();
};

#endif