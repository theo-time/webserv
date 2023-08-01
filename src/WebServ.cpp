/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jde-la-f <jde-la-f@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:03 by adcarnec          #+#    #+#             */
/*   Updated: 2023/08/01 11:59:57 by jde-la-f         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "Request.hpp"
#include "VirtualServer.hpp"
#include <ctime>
#include <sys/time.h>

typedef std::vector<VirtualServer*>         srvVect;
typedef std::map<int, Request*>             intCliMap;
typedef std::map<int, int>                  intMap;

static bool         clean(std::string& src);
static void         handleHeader(Request &request);
static bool         checkEndRequestHeader(const char* src, int size, int* pos);
static void         addChunkedBody(Request &request, std::string& requestRawString);
static bool         getRequestRawString(const int &fd, std::string& requestRawString);

fd_set                                      WebServ::_master_set_recv;
fd_set                                      WebServ::_master_set_write;
intCliMap                                   WebServ::_requests;
intMap                                      WebServ::_listeners;
int                                         WebServ::_max_fd;

bool WebServ::runListeners(void)
{

    if (Config::getVirtualServers().empty())
    {
        std::cout << "Error: no configured virtual server" << std::endl;
        return(false);
    }

    if (!init())
        return(false);

    if (!process())
        return(false);

    return(true);
}

bool WebServ::init(void)
{
    FD_ZERO(&_master_set_recv);
    FD_ZERO(&_master_set_write);
    
    _max_fd = 0;

    srvVect::iterator srvIt = Config::getVirtualServers().begin();
    srvVect::iterator srvEnd = Config::getVirtualServers().end();

    while(srvIt != srvEnd)
    {
        std::cout << "Starting virtual server " << (*srvIt)->getPort() << " " << (*srvIt)->getRoot() << "..." << std::endl;
        if (_listeners.count((*srvIt)->getPort())) 
        {
            (*srvIt)->setFd(_listeners[(*srvIt)->getPort()]);
            std::cout << "... binding to existing server socket - fd " << (*srvIt)->getFd() << std::endl;
        }
        else
        {            
            if (!runListener((*srvIt)))
                return(false);
        }
        srvIt++;
    }
    // check user cmd in terminal
    add(0, _master_set_recv);
    return(true);
}

bool WebServ::runListener(VirtualServer* srv)
{
    int     listenQ = 32; //maximum length to which the queue of pending connections for socket may grow

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd == -1)
        return(strerror(errno), false);
        
    // Allow socket descriptor to be reuseable 
    int on = 1;
    if (setsockopt(listenFd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) == -1)
        return(strerror(errno), close(listenFd), false);

    // Set socket to be nonblocking
    if (fcntl(listenFd, F_SETFL, O_NONBLOCK) == -1)
        return(strerror(errno), close(listenFd), false);

    struct sockaddr_in  srvAddr;
    // ft_bzero(&srvAddr, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_addr.s_addr = INADDR_ANY;
    srvAddr.sin_port = htons(srv->getPort());
    if (bind(listenFd, (struct sockaddr*)&srvAddr, sizeof(srvAddr)) == -1) // bind the socket to port number
        return(strerror(errno), close(listenFd), false);

    if (listen(listenFd, listenQ) == -1) // wait for connections
        return(strerror(errno), close(listenFd), false);
    std::cout << "... waiting for connexion: port " << srv->getPort() << std::endl;

    //add to fd lists
    srv->setFd(listenFd);
    add(listenFd, _master_set_recv);
    _listeners[srv->getPort()] = listenFd; 
    return(true);
}

bool WebServ::process(void)
{  
    int             ret;
    fd_set          working_set_recv;
    fd_set          working_set_write;
    struct timeval  timeout;

    while (true) {
        // initialize the timeval struct
        timeout.tv_sec  = 10;
        timeout.tv_usec = 0;

        // update fd_set
        prepSelect();
        working_set_recv = _master_set_recv;
        working_set_write = _master_set_write;

        // check i/o activity
        ret = select(_max_fd + 1, &working_set_recv, &working_set_write, NULL, &timeout);

        if (ret == - 1) {
            std::cerr << " select() failed" << std::endl;
            // stop();
            return(true);
        }
        if (ret == 0 && !_requests.empty()) {
            handleTimeout();
        }
        for (int i = 0; i <= _max_fd; ++i) {
            if (FD_ISSET(i, &working_set_recv) && i == 0) {
                if (userExit())
                    return(false);
            }
            else if (FD_ISSET(i, &working_set_recv) && isServerSocket(i)) 
                acceptNewCnx(i);
            else if (FD_ISSET(i, &working_set_recv) && _requests.count(i))
                readRequest(i, *_requests[i]);
            else if (FD_ISSET(i, &working_set_write) && _requests.count(i))
                sendResponse(i, *_requests[i]);
        }
    }
    return(true);
}

bool WebServ::acceptNewCnx(const int& fd)
{
    int                 clientSocket;
    struct sockaddr_in  clientAddress;
    long                clientAddressSize = sizeof(clientAddress);

    do {
        clientSocket = accept(fd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddressSize);
        if (clientSocket == -1)
            break;

        // Set socket to be reusable and nonblocking
        int on = 1;
        if (setsockopt(clientSocket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) == -1) {
            strerror(errno);
            close(clientSocket);
            break;
        }  
        if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1) {
            strerror(errno);
            close(clientSocket);
            break;
        }

        add(clientSocket, _master_set_recv);
        _requests[clientSocket] = new Request(clientSocket, fd);
        std::cout << "  New incoming connection - fd " << clientSocket << std::endl;
    } while (clientSocket != -1);

    return(true);
}

bool WebServ::readRequest(const int &fd, Request &request)
{
    std::cout << "  Reading request - fd " << fd << std::endl;
    std::string requestRawString("");
    if (!getRequestRawString(fd, requestRawString)){
        closeCnx(fd);
        return(true);
    }

    if (request.readingHeader){
        request.appendRequestString(requestRawString);
        std::cout << "  ***requestRawString added to existing request:" << std::endl << requestRawString << "***" << std::endl << std::endl;
    }
    else if (request.readingBody){
        if (request.chunkedBody){
            std::cout << "  ***added to requestBodyList:" << std::endl << requestRawString << "***" << std::endl << std::endl;
            addChunkedBody(request, requestRawString);
        }
        else {
            request.requestBodyString = request.requestBodyString + requestRawString;
            if (static_cast<int>(request.requestBodyString.size()) >= request.contentLength)
            {
                request.readingBody = false;
                if (static_cast<int>(request.requestBodyString.size()) > request.contentLength) 
                    request.requestBodyString.erase(request.contentLength);
                std::cout << "  ***end of request.requestBodyString:" << std::endl << request.requestBodyString << "***" << std::endl << std::endl;
                request.handleRequest();
            }
            else
                std::cout << "  ***added to request.requestBodyString:" << std::endl << requestRawString << "***" << std::endl << std::endl;
        }
    }
    else { 
        std::cout << "  ***new request:" << std::endl << requestRawString << "***" << std::endl << std::endl;
        if (!clean(requestRawString))
        {
            request.getResponse().sendError(400, ": invalid HTTP method");
            return(true);
        }
        if (requestRawString.empty())
            return(true);
        request.setRequestString(requestRawString);
        request.requestBodyString = "";
        request.readingHeader = true;
    }

    handleHeader(request);
    return(true);
}

bool WebServ::sendResponse(const int &fd, Request &request)
{
    std::cout << "  Sending response - fd " << fd << std::endl;
    std::cout << request.getResponseString() << std::endl;
    int rc  = send(fd, request.getResponseString().c_str(), request.getResponseString().length(), 0);
    request.clear();

    std::cout << "  *** check RC : " << rc << std::endl;
    if (rc == -1 || rc == 0) {
        std::cerr << "  send() failed" << std::endl;
        closeCnx(fd);
        del(fd, _master_set_write);
    }
    else {
        del(fd, _master_set_write);
        add(fd, _master_set_recv);
    }
    
    return(true);
}

void WebServ::add(const int& fd, fd_set& set)
{
    FD_SET(fd, &set);

    if (fd > _max_fd)
        _max_fd = fd;
}

void WebServ::addFd2Select(const int& fd)
{
    FD_SET(fd, &_master_set_recv);

    if (fd > _max_fd)
        _max_fd = fd;
}

void WebServ::delFd2Select(const int& fd)
{
    FD_CLR(fd, &_master_set_recv);

    if (fd == _max_fd)
    {
        while (FD_ISSET(_max_fd, &_master_set_recv) == 0)
        _max_fd--;
    }
}

void WebServ::del(const int& fd, fd_set& set)
{
    FD_CLR(fd, &set);

    if (fd == _max_fd)
    {
        while (FD_ISSET(_max_fd, &set) == 0)
        _max_fd--;
    }
}

void WebServ::closeCnx(const int& fd)
{
    std::cout << "  Closing cnx - fd " << fd << std::endl;
    if (FD_ISSET(fd, &_master_set_recv))
    {        
        del(fd, _master_set_recv);
    }

    if (FD_ISSET(fd, &_master_set_write))
    {        
        del(fd, _master_set_write);
    }

    close(fd);
    delete _requests.at(fd);
    _requests.erase(fd);
}

void WebServ::stop(void)
{

    Config::clear();

    intCliMap::iterator      cliIt  = _requests.begin();
    while(cliIt != _requests.end())
    {
        Request* tmp = cliIt->second;
        delete tmp;
        cliIt++;
    }
    _requests.clear();

    for (int i = 3; i <= _max_fd; ++i)
    {
        if (FD_ISSET(i, &_master_set_recv)) 
            close(i);
        else if (FD_ISSET(i, &_master_set_write))
            close(i);
    }
}

bool WebServ::isServerSocket(const int& fd)
{
    intMap::iterator lsIt        = _listeners.begin();
    intMap::iterator lsEnd       = _listeners.end();

    while (lsIt != lsEnd)
    {
        if (lsIt->second == fd)
            return(true);
        lsIt++;
    }
    return(false);
}

bool WebServ::userExit(void)
{
    std::string buffer;
    std::getline(std::cin, buffer);

    if (std::cin.eof() || buffer == "EXIT")
        return(true);

    return(false);
}

void WebServ::prepSelect(void)
{
    std::cout << "Preparing select" << std::endl;
    // print server sockets
    for (int i = 0; i <= _max_fd; ++i)
    {
        if (FD_ISSET(i, &_master_set_recv) && isServerSocket(i)) 
            std::cout << "  fd - " << i << " : server socket" << std::endl;
    }

    if (!_requests.empty())
    {            
        std::cout << "  Existing client sockets:" << std::endl;
        
        // add responses to _master_set_write
        for (int i = 0; i <= _max_fd; ++i)
        {
            if (_requests.count(i))
            {
                std::cout << "      fd - " << i  << ", curRequestTime=" << _requests[i]->curRequestTime << std::endl;
                if (_requests[i]->ready2send)
                {
                    del(i, _master_set_recv);
                    add(i, _master_set_write);
                }
            }
        }

        // print client sockets
        for (int i = 0; i <= _max_fd; ++i)
        {
            if (FD_ISSET(i, &_master_set_recv))
                std::cout << "  fd - " << i << " : working_set_recv" << std::endl;
            else if (FD_ISSET(i, &_master_set_write))
                std::cout << "  fd - " << i << " : working_set_write" << std::endl;
            else
                std::cout << "  fd - " << i << " : not working set" << std::endl;
        }
    }
}

void WebServ::handleTimeout(void)
{
    if (!_requests.empty())
    {
        intCliMap::iterator it  = _requests.begin();
        while(it != _requests.end())
        {
            if (it->second->curRequestTime > 0)
            {
                if (ft_now() >= it->second->curRequestTime + Config::requestTimeout)
                {
                    std::cout << "TIMEOUT FOR REQUEST fd - " << it->first << std::endl;
                    it->second->getResponse().sendError(408, "Timeout");
                }
            }
            it++;
        }
    }
}

static bool getRequestRawString(const int &fd, std::string& requestRawString)
{
    char        buffer[BUFFER_SIZE];
    int         rc = 0;
    
    int i = -1;
    while (++i < BUFFER_SIZE)
        buffer[i] = 0;

    while (true)
    {
        rc = recv(fd, buffer, sizeof(buffer), 0);
        std::cout << "  " << rc << " bytes received" << std::endl;
        if (rc == -1)
        {
            strerror(errno);
            break;
        }
        if (rc == 0)
        {
            std::cerr << "  Connection closed\n" << std::endl;
            return(false);
        }
        buffer[rc] = 0;
        requestRawString.append(buffer);
    }
    return(true);
}

static void handleHeader(Request &request)
{    
    int posBodyStart = -1;
    if (request.readingHeader && checkEndRequestHeader(request.getRequestString().c_str(), request.getRequestString().size(), &posBodyStart))
    {
        std::cout << "  " << request.getRequestString().size() << " total bytes received\n  ***************\n" << std::endl;

        request.requestHeaderString = request.getRequestString().substr(0, posBodyStart);
        request.requestBodyString = request.getRequestString().substr(posBodyStart);

        std::cout << "  ***requestHeaderString:"  << std::endl << request.requestHeaderString << "***" << std::endl;
        std::cout << "  ***requestBodyString:"  << std::endl << request.requestBodyString << "***" << std::endl;
        
        request.readingHeader = false;
        if (!request.parseRequest())
            return;
        request.curRequestTime = ft_now();

        if (request.contentLength != -1)
        {
            std::cout << "Expected Content-Length   = " << request.contentLength << std::endl;
            if (static_cast<int>(request.requestBodyString.size()) < request.contentLength)
                request.readingBody = true;
        }
        else if (request.getHeader("Transfer-Encoding") == "chunked" )
        {
            std::cout << "CHUNKED = TRUE" << std::endl;
            request.readingBody = true;
            request.chunkedBody = true;

            if (!request.requestBodyString.empty())
                addChunkedBody(request, request.requestBodyString);
        }

        if (request.readingBody == false)
            request.handleRequest();
    }
}

static bool checkEndRequestHeader(const char* src, int size, int* pos)
{
    int i = 0;
    while (i < size - 3)
    {
        if (src[i] == '\r' && src[i + 1] == '\n' && src[i + 2] == '\r' && src[i + 3] == '\n')
            return(*pos = i + 4, true);
        i++;
    }
    return(false);
}

static void addChunkedBody(Request &request, std::string& requestRawString)
{
    if (!request.readingBody)
        return;

    if (requestRawString.empty())
        return;

    char* pEnd;
    if (request.curChunkSize == -1 || static_cast<int>(request.requestBodyList.back().size()) == request.curChunkSize)
    {
        request.curChunkSize = strtoul(requestRawString.c_str(), &pEnd, 16);
        std::cout << "CHUNK SIZE = " << request.curChunkSize << std::endl; 
        if (request.curChunkSize != 0)
        {
            requestRawString = pEnd + 2;
            request.requestBodyList.push_back("");
        }
    }

    if (request.curChunkSize == 0)
    {
        std::cout << "END OF CHUNKED BODY: TOTAL SIZE=" << request.getChunkedBodySize() << std::endl; 
        request.readingBody = false;
        request.handleRequest();
        return;
    }

    std::cout << "EXTRACTING BODY... request.curChunkSize=" << request.curChunkSize << " request.requestBodyList.back().size()=" << request.requestBodyList.back().size() << " requestRawString.size()=" << requestRawString.size() << std::endl;
    int missingData = request.curChunkSize - static_cast<int>(request.requestBodyList.back().size());
    if (static_cast<int>(requestRawString.size()) <= missingData)
    {
        request.requestBodyList.back().append(requestRawString);
        requestRawString.clear();
    }
    else
    {
        request.requestBodyList.back().append(requestRawString.substr(0, missingData));
        requestRawString.erase(0, missingData);
        std::cout << "requestBodyList.back = " << request.requestBodyList.back() << std::endl; 
    }

    if (requestRawString.empty())
        return;

    std::cout << "remaining requestRawString = " << requestRawString << std::endl;
    addChunkedBody(request, requestRawString);
}

/*
GET
The GET method requests a representation of the specified resource. Requests using GET should only retrieve data.

HEAD
The HEAD method asks for a response identical to a GET request, but without the response body.

POST
The POST method submits an entity to the specified resource, often causing a change in state or side effects on the server.

PUT
The PUT method replaces all current representations of the target resource with the request payload.

DELETE
The DELETE method deletes the specified resource.

CONNECT
The CONNECT method establishes a tunnel to the server identified by the target resource.

OPTIONS
The OPTIONS method describes the communication options for the target resource.

TRACE
The TRACE method performs a message loop-back test along the path to the target resource.

PATCH
The PATCH method applies partial modifications to a resource.
*/

const std::string   WebServ::httpMethods[9] = {"GET ", "HEAD ", "POST ", "PUT ", "DELETE ", "CONNECT ", "OPTIONS ", "TRACE ", "PATCH "};
static bool clean(std::string& src)
{
    size_t found;
    int i = 0;
    while (i < 9)
    {
        found = src.find(WebServ::httpMethods[i]);
        if (found != std::string::npos)
        {
            src.erase(0, found); // enlever juste whitespaces 
            return(true);
        }
        i++;
    }

    //trim
    std::string::iterator it = src.begin();
    while (it != src.end() && (*it == '\n' || *it == ' ' || (*it >= 9 && *it <= 13)))
    {
        src.erase(it);
        // it++;
    }
    if (src.empty())
        return(true);

    return(false);
}
