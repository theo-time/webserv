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
#include <cstdlib>
#include <list>

# include "WebServ.hpp"
# include "VirtualServer.hpp"
# include "Location.hpp"
# include "Response.hpp"
# include "Utils.hpp"

// TYPES 
#define GET 1
#define POST 2
#define DELETE 3
#define UNKNOWN 0

#define MAX_URI_LEN 100
#define MAX_HEADER_LEN 5000

#define CGI_BUFFER_SIZE 2046

class Request {
    private:
        /* Header */
        std::string method;
        std::string path;
        std::string originalPath;
        std::string protocol;
        std::string requestString;
        std::string body;
        std::string query;
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

        std::string header;
        std::string executable_path;
        std::string script_path;

        bool                    chunkedBody;
        bool                    readingBody;
        bool                    readingHeader;
        std::string             requestBodyString;
        std::string             requestHeaderString;
        std::list<std::string>  requestBodyList;
        int                     curChunkSize; 
        int                     contentLength;
	    unsigned long	        curRequestTime;
	    unsigned long	        lastActivityTime;

        bool                    ready2send;

        /* Constructors & Destructors */
        Request();
        Request(int clientSocket, int serverSocket);
        ~Request();

        /* Getters & Setters */
        std::string         getPath();
        std::string         getProtocol();
        std::string         getMethod();
        std::string         getBody();
        std::string         getQuery();
        int                 getMethodCode();
        std::string         getRequestString();
        int                 getClientSocket();
        int                 getServerSocket();
        std::string         getResponseString();
        std::string         getHeader(std::string key);
        void                getRequestConfig();
        Location*           getConfig();
        Response&           getResponse(){return _response;}
        void                setFileContent(std::string &fileContent);
        void                setResponseString(std::string &response);
        void                setRequestString(std::string &request);
        void                appendRequestString(std::string request);
        void                setConfig(Location* config);

        /* Methods */

        void routingGet();
        void routingPost();
        void routingDelete();
        void routingCGI();
        bool parseRequest();
        void parseMethodToken(const std::string& token);
        bool parseHTTPVersion(const std::string& token);
        bool parseHeaders();
        bool parseBody();
        bool parseURI(std::string token);
        void handleRequest();
        void buildResponse();
        bool fileExists();
        void listDirectoryResponse();
        void clear();
        int  getChunkedBodySize();
};

#endif
