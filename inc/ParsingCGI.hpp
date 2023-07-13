#ifndef PARSINGCGI_HPP
#define PARSINGCGI_HPP 

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include "Request.hpp"
#include "CGI.hpp"

class ParsingCGI {
    private:
        std::string method;
        std::string path;
        std::string body;
        std::string protocol;

    public:

        // std::unordered_map<std::string, std::string> headers;
        std::map<std::string, std::string> headers;

        ParsingCGI();
        ~ParsingCGI();

        void parseRequest(const std::string& request);

        std::string getPath();
        std::string getProtocol();
        std::string getMethod();
        std::string getBody();
};

#endif