/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jde-la-f <jde-la-f@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:03 by adcarnec          #+#    #+#             */
/*   Updated: 2023/07/27 15:17:17 by jde-la-f         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <list>

#include "WebServ.hpp"
#include "Request.hpp"
#include "VirtualServer.hpp"

typedef std::vector<VirtualServer*>         srvVect;
typedef std::map<int, Request*>             intCliMap;
typedef std::map<int, int>                  intMap;
fd_set                                      WebServ::_master_set_recv;
fd_set                                      WebServ::_master_set_write;
int                                         WebServ::_max_fd;
intCliMap                                   WebServ::_requests;
intMap                                      WebServ::_listeners;

bool WebServ::runListeners(void)
{

    if (Config::getVirtualServers().empty()) // TODO port par defaut ?
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
        if (_listeners.count((*srvIt)->getPort())) // TODO prendre en compte host?
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

    while (true)
    {
        prepSelect();
        working_set_recv = _master_set_recv;
        working_set_write = _master_set_write;

        // TODO check socket errors
        ret = select(_max_fd + 1, &working_set_recv, &working_set_write, NULL, 0);

        if (ret == - 1)
        {
            std::cerr << "  select() failed" << std::endl;
            stop();
            return(false);
        }
        if (ret == 0)
        {
            std::cerr << "  select() timed out.  End program.\n" << std::endl;
            stop();
            return(false);
        }
        for (int i = 0; i <= _max_fd; ++i)
        {
            if (FD_ISSET(i, &working_set_recv) && i == 0) 
            {
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

void WebServ::prepSelect(void)
{
    // TODO check request timeout
    
    //add responses to _master_set_write
    if (!_requests.empty())
    {            
        for (int i = 0; i <= _max_fd; ++i)
        {
            if (_requests.count(i) && _requests[i]->ready2send)
            {
                del(i, _master_set_recv);
                add(i, _master_set_write);
            }
        }
    }

    // TODO cout à supprimer
    std::cout << "Preparing select() " << std::endl;
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

bool WebServ::acceptNewCnx(const int& fd)
{
    int                 clientSocket;
    struct sockaddr_in  clientAddress;
    long                clientAddressSize = sizeof(clientAddress);

    do
    {
        clientSocket = accept(fd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddressSize);
        if (clientSocket == -1)
            break;

        // Set socket to be reusable and nonblocking
        int on = 1;
        if (setsockopt(clientSocket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) == -1)
        {
            strerror(errno);
            close(clientSocket);
            break;
        }  
        if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1)
        {
            strerror(errno);
            close(clientSocket);
            break;
        }

        // TODO set receive SO_RCVTIMEO and send timeout  SO_SNDTIMEO
        
        add(clientSocket, _master_set_recv);
        _requests[clientSocket] = new Request(clientSocket, fd);
        std::cout << "  New incoming connection - fd " << clientSocket << std::endl;
    } while (clientSocket != -1);

    return(true);
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

static std::string clean(std::string src)
{
    size_t found = src.find("GET");
    if (found != std::string::npos)
        return(src.substr(found));

    found = src.find("POST");
    if (found != std::string::npos)
        return(src.substr(found));

    found = src.find("HEAD");
    if (found != std::string::npos)
        return(src.substr(found));

    found = src.find("PUT");
    if (found != std::string::npos)
        return(src.substr(found));

    found = src.find("DELETE");
    if (found != std::string::npos)
        return(src.substr(found));

    return(src);
}

static void addChunkedBody(Request &request, std::string& requestRawString)
{
    if (!request.readingBody)
        return;

    char* pEnd;
    if (request.curChunkSize == -1 || (int)request.requestBodyList.back().size() == request.curChunkSize)
    {
        request.curChunkSize = strtol(requestRawString.c_str(), &pEnd, 16);
        std::cout << "CHUNK SIZE = " << request.curChunkSize << std::endl; 
        // TODO check pEnd
        if (request.curChunkSize != 0)
        {
            requestRawString = pEnd + 2;
            request.requestBodyList.push_back("");
        }
    }

    if (request.curChunkSize == 0)
    {
        std::cout << "END OF CHUNKED BODY" << std::endl; 
        request.readingBody = false;
        request.handleRequest();
        return;
    }

    std::cout << "EXTRACTING BODY... request.curChunkSize=" << request.curChunkSize << " request.requestBodyList.back().size()=" << request.requestBodyList.back().size() << " requestRawString.size()=" << requestRawString.size() << std::endl;
    int missingData = request.curChunkSize - (int)request.requestBodyList.back().size();
    if ((int)requestRawString.size() <= missingData)
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

bool WebServ::readRequest(const int &fd, Request &request)
{
    std::cout << "  Reading request - fd " << fd << std::endl;
    char        buffer[BUFFER_SIZE];
    int         rc = 0;
    
    int i = -1;
    while (++i < BUFFER_SIZE)
        buffer[i] = 0;

    std::string requestRawString("");
    while (true)
    {
        rc = recv(fd, buffer, sizeof(buffer), 0);
        std::cout << "  " << rc << " bytes received" << std::endl;
        if (rc < 0)
            break;
        if (rc == 0)
        {
            std::cerr << "  Connection closed\n" << std::endl;
            closeCnx(fd);
            return(true);
        }
        buffer[rc] = 0;
        requestRawString.append(buffer);
    }

    if (request.readingHeader)
    {
        request.appendRequestString(requestRawString);
        std::cout << "  ***requestRawString added to existing request:" << std::endl << requestRawString << "***" << std::endl << std::endl;
    }
    else if (   requestRawString.find("GET") != std::string::npos 
            ||  requestRawString.find("POST") != std::string::npos 
            ||  requestRawString.find("HEAD") != std::string::npos 
            ||  requestRawString.find("PUT") != std::string::npos
            ||  requestRawString.find("DELETE") != std::string::npos)
    {
        request.appendRequestString(clean(requestRawString));
        std::cout << "  ***new valid request:" << std::endl << requestRawString << "***" << std::endl << std::endl;
        request.requestBodyString = "";
        request.readingHeader = true;
    }
    else
    {
        if (request.chunkedBody && request.readingBody)
        {

            std::cout << "  ***added to requestBodyList:" << std::endl << requestRawString << "***" << std::endl << std::endl;
            addChunkedBody(request, requestRawString);
        }
        else
        {
            request.requestBodyString = request.requestBodyString + requestRawString;
            std::cout << "  ***added to request.requestBodyString:" << std::endl << requestRawString << "***" << std::endl << std::endl;
        }
        
    }
    
    int posBodyStart = -1;
    if (request.readingHeader && checkEndRequestHeader(request.getRequestString().c_str(), request.getRequestString().size(), &posBodyStart))
    {
        std::cout << "  " << request.getRequestString().size() << " total bytes received\n  ***************\n" << std::endl;

        request.requestHeaderString = request.getRequestString().substr(0, posBodyStart);
        request.requestBodyString = request.getRequestString().substr(posBodyStart);

        std::cout << "  ***requestHeaderString:" << request.requestHeaderString << "***" << std::endl;
        std::cout << "  ***requestBodyString:" << request.requestBodyString << "***" << std::endl;
        
        request.readingHeader = false;
 /*        if (request.getRequestString().find("GET") == 0 || request.getRequestString().find("POST") == 0 ||
         request.getRequestString().find("HEAD") == 0 || request.getRequestString().find("PUT") == 0) 
        { */
        request.parseRequest();
        
        if (request.getHeader("Transfer-Encoding") == "chunked")
        {
            std::cout << "CHUNKED = TRUE" << std::endl;
            request.readingBody = true;
            request.chunkedBody = true;

            if (!request.requestBodyString.empty())
                addChunkedBody(request, request.requestBodyString);
        }
        if (request.readingBody == false)
            request.handleRequest();
/*         }
        else
           request.clear(); */
    }
        
    return(true);
}

bool WebServ::sendResponse(const int &fd, Request &c)
{
    std::cout << "  Sending response - fd " << fd << std::endl;
    std::cout << c.getResponseString() << std::endl;
    int rc  = send(fd, c.getResponseString().c_str(), c.getResponseString().length(), 0);
    c.clear();
    std::cout << "  *** check RC : " << rc << std::endl;
    if (rc < 0)
    {
        std::cerr << "  send() failed" << std::endl;
        closeCnx(fd);
        del(fd, _master_set_write);
    }
    else
    {
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

    if (buffer == "EXIT")
        return(true);

    return(false);
}
