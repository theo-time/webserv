/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:03 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:05 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

typedef std::vector<VirtualServer*>         srvVect;
typedef std::map<int, ClientCnx*>           intCliMap;
typedef std::map<ClientCnx*, std::string>   cliStrMap;
typedef std::map<int, int>                  intMap;
typedef std::map<int, std::string>          intStrMap;
fd_set                                      WebServ::_master_set_recv;
fd_set                                      WebServ::_master_set_write;
int                                         WebServ::_max_fd;
intCliMap                                   WebServ::_clients;
cliStrMap                                   WebServ::_reqRessources;
intMap                                      WebServ::_listeners;
intStrMap                                   WebServ::_fdRessources;

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
            // Create server socket to receive incoming connections on
            int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (serverSocket == -1) {
                std::cerr << "Error : socket creation failed" << std::endl;
                return(false);
            }
            // Allow socket descriptor to be reuseable 
            int on = 1;
            int rc = setsockopt(serverSocket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
            if (rc < 0)
            {
                std::cerr << "Error : setting socket options failed" << std::endl;
                close(serverSocket);
                return(false);
            }
            // Set socket to be nonblocking.  
            rc = fcntl(serverSocket, F_SETFL, O_NONBLOCK);
            if (rc < 0)
            {
                std::cerr << "Error : setting nonblocking option failed" << std::endl;
                close(serverSocket);
                return(false);
            }
            // Bind the socket
            struct sockaddr_in  serverAddress;
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = INADDR_ANY;
            serverAddress.sin_port = htons((*srvIt)->getPort()); 
            rc = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
            if (rc < 0)
            {
                std::cerr << "Error : socket binding failed" << std::endl;
                close(serverSocket);
                return(false);
            }

            // Set the listen back log
            rc = listen(serverSocket, 32);
            if (rc < 0)
            {
                std::cerr << "Error : socket listen failed" << std::endl;
                close(serverSocket);
                return(false);
            }
            std::cout << "... waiting for connexion: port " << (*srvIt)->getPort() << std::endl;
            (*srvIt)->setFd(serverSocket);
            add(serverSocket, _master_set_recv);
            _listeners[(*srvIt)->getPort()] = serverSocket; 
        }
        srvIt++;
    }
    return(true);
}

bool WebServ::process(void)
{
    struct timeval  timeout;    
    int             ret;
    fd_set          working_set_recv;
    fd_set          working_set_write;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    while (true)
    {
        working_set_recv = _master_set_recv;
        working_set_write = _master_set_write;

        // TODO cout à supprimer
        std::cout << "Preparing select() " << std::endl;
        for (int i = 0; i <= _max_fd; ++i)
        {
            if (FD_ISSET(i, &working_set_recv))
                std::cout << "  fd - " << i << " : working_set_recv" << std::endl;
            else if (FD_ISSET(i, &working_set_write))
                std::cout << "  fd - " << i << " : working_set_write" << std::endl;
            else
                std::cout << "  fd - " << i << " : not working set" << std::endl;
        }

        ret = select(_max_fd + 1, &working_set_recv, &working_set_write, NULL, &timeout);
        if (ret < 0)
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
            if (FD_ISSET(i, &working_set_recv) && isServerSocket(i)) 
                acceptNewCnx(i);
            else if (FD_ISSET(i, &working_set_recv) && _fdRessources.count(i))
                readRessource(i);
            else if (FD_ISSET(i, &working_set_recv) && _clients.count(i))
                readRequest(i, *_clients[i]);
            else if (FD_ISSET(i, &working_set_write) && _clients.count(i))
                sendResponse(i, *_clients[i]);
        }
    }
    return(true);
}

bool WebServ::acceptNewCnx(const int& fd)
{
    int                 clientSocket;
    struct sockaddr_in  clientAddress;
    long                clientAddressSize = sizeof(clientAddress);

    do
    {
        clientSocket = accept(fd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddressSize);
        if (clientSocket < 0)
        {
            if (errno != EWOULDBLOCK) // TODO ne pas utiliser errno
            {
                std::cerr << "  accept() failed" << std::endl;
                stop();
                return(false);
            }
            break;
        }

        // Set socket to be reusable and nonblocking
        int on = 1;
        int rc = setsockopt(clientSocket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
        if (rc < 0)
        {
            std::cerr << "Error : setting reusable option failed" << std::endl;
            close(clientSocket);
            break;
        }  
        rc = fcntl(clientSocket, F_SETFL, O_NONBLOCK);
        if (rc < 0)
        {
            std::cerr << "Error : setting nonblocking option failed" << std::endl;
            close(clientSocket);
            break;
        }
        
        add(clientSocket, _master_set_recv);
        _clients[clientSocket] = new ClientCnx(clientSocket, clientAddress);
        std::cout << "  New incoming connection - fd " << clientSocket << std::endl;
    } while (clientSocket != -1);

    return(true);
}

bool WebServ::readRequest(const int &fd, ClientCnx &c)
{
    std::cout << "  Reading request - fd " << fd << std::endl;
    char        buffer[BUFFER_SIZE];
    int         rc = 0;
    
    buffer[0] = 0;

    std::string request("");
    while (true)
    {
        rc = recv(fd, buffer, sizeof(buffer), 0);
        if (rc < 0)
        {
            if (errno != EWOULDBLOCK) // TODO ne pas utilier errno
            {
                std::cerr << "  recv() failed" << std::endl;
                closeCnx(fd);
                // return(false) ???
            }
            break;
        }
        if (rc == 0)
        {
            std::cerr << "  Connection closed\n" << std::endl;
            closeCnx(fd);
            return(true);
        }
        buffer[rc] = 0;
        request.append(buffer);
    }

    std::cout << "  " << request.size() << " bytes received\n***************\n" << std::endl;
    c.newRequest(request);
    memset(buffer, 0, sizeof(buffer)); // TODO ft_memset
    del(fd, _master_set_recv);
    add(fd, _master_set_write);

    return(true);
}

bool WebServ::sendResponse(const int &fd, ClientCnx &c)
{
    std::cout << "  Sending response - fd " << fd << std::endl;
    int rc  = send(fd, c.getResponse().c_str(), c.getResponse().length(), 0);
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

bool WebServ::readRessource(const int& fd)
{
    std::cout << "Reading ressource from fd - " << fd << std::endl;

    std::string     fileContent = "";
    char            buf[BUFFER_SIZE + 1];
    int             ret = BUFFER_SIZE;

    buf[0] = 0;
    while (ret == BUFFER_SIZE)
    {
        ret = read(fd, buf, BUFFER_SIZE);
        buf[ret] = 0;
        fileContent = fileContent + buf;
    }
    close(fd);
    del(fd, _master_set_recv);
    
    if (ret == -1)
    {
        // TODO fileContent = msg erreur
    }

    cliStrMap::iterator     it = _reqRessources.begin();
    while (it != _reqRessources.end())
    {
        if (it->second == _fdRessources[fd])
        {
            it->first->setFileContent(fileContent);
        }
        it++;
    }
    
    it = _reqRessources.begin();
    while (_reqRessources.size() > 0 && it != _reqRessources.end())
    {
        if (it->second == _fdRessources[fd])
        {
            _reqRessources.erase(it);
        }
        else
            it++;
    }
    _fdRessources.erase(fd);

    return(true);
}

void WebServ::getRessource(const std::string& path, ClientCnx& c)
{
    if (!isListedRessource(path))
    {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd == -1)
        {
            return;
        }
        add(fd, _master_set_recv);
        _fdRessources[fd] = path;
    }
    _reqRessources[&c] = path;
}

bool WebServ::isListedRessource(const std::string& path)
{              
    intStrMap::iterator  it = _fdRessources.begin();
    while (it != _fdRessources.end())
    {
        if (it->second == path)
            return(true);
        it++;
    }
    return(false);
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
    delete _clients.at(fd);
    _clients.erase(fd);
}

void WebServ::stop(void)
{

    //TODO close sockets

    Config::clear();

    intCliMap::iterator      cliIt  = _clients.begin();
    intCliMap::iterator      cliEnd  = _clients.end();
    while(cliIt != cliEnd)
    {
        ClientCnx* tmp = cliIt->second;
        delete tmp;
        cliIt++;
    }
    _clients.clear();
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
