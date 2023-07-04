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
#include <sys/types.h>
#include <dirent.h>


# include "WebServ.hpp"
# include "Location.hpp"
# include "Response.hpp"

// TYPES 
#define GET 1
#define POST 2
#define DELETE 3
#define UNKNOWN 0

class Request {
    private:
        /* Header */
        std::string method;
        std::string path;
        std::string protocol;
        std::string requestString;
        std::string requestString2;
        std::string header;
        std::string body;
        std::string responseString;
        std::string fileContent;
        std::map<std::string, std::string> headers;

        int methodCode;
        bool cgi_mode;

        Location*       _config;
        Response            _response;  
        int clientSocket;
        int serverSocket;

    public:
        /* Constructors & Destructors */
        Request();
        Request(int clientSocket, int serverSocket);
        ~Request();

        /* Getters & Setters */
        int getType();
        std::string getPath();
        std::string getProtocol();
        std::string getMethod();
        std::string getRequestString();
        int         getClientSocket();
        int         getServerSocket();
        std::string getResponseString();
        std::string getHeader(std::string key);
        Location* getConfig();
        void setFileContent(std::string &fileContent);
        void setResponseString(std::string &response);
        void setRequestString(std::string &request);
        void setConfig(Location* config);

        /* Methods */

        void get();
        void post();
        void mdelete();
        void parseRequest();
        void handleRequest();
        void buildResponse();
        bool fileExists();
        void retrieveHeaderAndBody(const std::string& input);
};

#endif