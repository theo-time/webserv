#ifndef RESPONSE_HPP
#define RESPONSE_HPP 


#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

class Request {
    private:
        int clientSocket;
        std::string protocol;
        std::string statusMessage;
        std::string contentType;
        std::string content;
        int statusCode;

    public:
        Response(int clientSocket, std::string request);
        ~Request();

};

#endif