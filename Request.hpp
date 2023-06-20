#ifndef REQUEST_HPP
#define REQUEST_HPP 


#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

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
        std::string request;
        std::string response;
        std::string extension;
        std::string outputCGI;
        int type;

    public:
        Request(int clientSocket, std::string request);
        ~Request();

        int getType();
        std::string getPath();
        std::string getProtocol();
        std::string getMethod();
        std::string getRequest();
        int getClientSocket();
        std::string getResponse();

        void prepareResponse();
        bool fileExists();
};

#endif