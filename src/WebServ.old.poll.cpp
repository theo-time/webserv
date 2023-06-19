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
typedef std::map<int, Client*>              cliMap;
typedef std::map<int, int>                  listenMap;
fd_set                                      WebServ::_master_set_recv;
fd_set                                      WebServ::_master_set_write;
int                                         WebServ::_max_fd;
cliMap                                      WebServ::_clients;
listenMap                                   WebServ::_listeners;
struct pollfd WebServ::fds[200];

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

    std::cout << "Config::getVirtualServers().size " << Config::getVirtualServers().size() << std::endl;

    while(srvIt != srvEnd)
    {
        std::cout << "Starting virtual server " << (*srvIt)->getPort() << " " << (*srvIt)->getRoot() << std::endl;
        if (_listeners.count((*srvIt)->getPort())) // TODO prendre en compte host
        {
            std::cout << "Binding " << (*srvIt)->getPort() << " to existing server socket" << std::endl;
            (*srvIt)->setFd(_listeners[(*srvIt)->getPort()]);
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
                perror("server socket binding ");
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
            std::cout << "Waiting for connexion: port " << (*srvIt)->getPort() << "..." << std::endl;
            (*srvIt)->setFd(serverSocket);
            // add(serverSocket, _master_set_recv);
            _listeners[(*srvIt)->getPort()] = serverSocket; 
        }
        srvIt++;
    }
    return(true);
}

bool WebServ::process(void)
{
//   char   buffer[1024];
 int rc = 0;
 int nfds = _listeners.size() + _clients.size();
 int current_size = 0;
//  int new_sd = -1;
 int i;
  /*************************************************************/
  /* Initialize the pollfd structure                           */
  /*************************************************************/
  memset(fds, 0 , sizeof(fds));

  /*************************************************************/
  /* Set up the initial listening socket                       */
  /*************************************************************/
listenMap::iterator it  = _listeners.begin();
listenMap::iterator end = _listeners.end();
int j = 0;
while (it != end)
{
  fds[j].fd = it->second;
  fds[j].events = POLLIN;
  j++;
    it++;
}

  /*************************************************************/
  /* Initialize the timeout to 3 minutes. If no                */
  /* activity after 3 minutes this program will end.           */
  /* timeout value is based on milliseconds.                   */
  /*************************************************************/
//  int timeout = (3 * 60 * 1000);
 int timeout = (10 * 1000);

  /*************************************************************/
  /* Loop waiting for incoming connects or for incoming data   */
  /* on any of the connected sockets.                          */
  /*************************************************************/
 while(true)
 {
    /***********************************************************/
    /* Call poll() and wait 3 minutes for it to complete.      */
    /***********************************************************/
    printf("Waiting on poll()...\n");
    rc = poll(fds, nfds, timeout);

    /***********************************************************/
    /* Check to see if the poll call failed.                   */
    /***********************************************************/
    if (rc < 0)
    {
      perror("  poll() failed");
      break;
    }

    /***********************************************************/
    /* Check to see if the 3 minute time out expired.          */
    /***********************************************************/
    if (rc == 0)
    {
      printf("  poll() timed out.  End program.\n");
      break;
    }


    /***********************************************************/
    /* One or more descriptors are readable.  Need to          */
    /* determine which ones they are.                          */
    /***********************************************************/
    current_size = nfds;
    printf("current_size = %i\n", current_size);
    for (i = 0; i < current_size; i++)
    {
        printf("fds[%i].fd = %i %s\n", i, fds[i].fd);

      if(fds[i].revents == 0)
        continue;

      if(fds[i].revents == POLLIN)
      {
        if (_listeners.count(fds[i].fd))
        {
            printf("  Listening socket is readable\n");
            acceptNewCnx(i);

        }
        else if (_clients.count(fds[i].fd))
        {
            printf("  Descriptor %d is readable\n", fds[i].fd);
            readRequest(i, *_clients[fds[i].fd]);
        }
      }
      else if (fds[i].revents == POLLOUT && _clients.count(fds[i].fd))
      {
            printf("  Descriptor %d is writable\n", fds[i].fd);
            sendResponse(i, *_clients[fds[i].fd]);
      }
      /*********************************************************/
      /* If revents is not POLLIN nor POLLOUT, it's an         */
      /* unexpected result, log and end the server.            */
      /*********************************************************/
      else
      {
        printf("  Error! revents = %d\n", fds[i].revents);
        return(false);
        break;
      }
    }
 } 

    return(true);
}

bool WebServ::acceptNewCnx(const int& i)
{
    int                 clientSocket;
    struct sockaddr_in  clientAddress;
    long                clientAddressSize = sizeof(clientAddress);

    do
    {
        clientSocket = accept(fds[i].fd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddressSize);
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
        
          fds[_listeners.size() + _clients.size()].fd = clientSocket;
          fds[_listeners.size() + _clients.size()].events = POLLIN;
        _clients[clientSocket] = new Client(clientSocket, clientAddress);
        std::cout << "  New incoming connection - fd " << clientSocket << std::endl;
    } while (clientSocket != -1);

    return(true);
}

bool WebServ::readRequest(const int &i, Client &c)
{
    std::cout << "  Reading request - fd " << fds[i].fd << std::endl;
    char        buffer[1024];
    int         rc = 0;
    
    buffer[0] = 0;

    std::string request("");
    while (true)
    {
        rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
        if (rc < 0)
        {
            if (errno != EWOULDBLOCK) // TODO ne pas utilier errno
            {
                std::cerr << "  recv() failed" << std::endl;
                closeCnx(fds[i].fd);
                // return(false) ???
            }
            break;
        }
        if (rc == 0)
        {
            std::cerr << "  Connection closed\n" << std::endl;
            closeCnx(fds[i].fd);
            return(true);
        }
        buffer[rc] = 0;
        request.append(buffer);
    }

    std::cout << "  " << request.size() << " bytes received\n***************\n" << std::endl;
    c.newRequest(request);
    memset(buffer, 0, sizeof(buffer)); // TODO ft_memset
    fds[i].events = POLLOUT;

    return(true);
}

bool WebServ::sendResponse(const int &i, Client &c)
{
    std::cout << "  Sending response - fd " << fds[i].fd << std::endl;
    char response[]  = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";

    (void)c; 
    int rc  = send(fds[i].fd, response, sizeof(response), 0);
    if (rc < 0)
    {
        std::cerr << "  send() failed" << std::endl;
        closeCnx(fds[i].fd);
    }
    else
    {
        fds[i].events = POLLIN;
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
    delete _clients.at(fd);
    _clients.erase(fd);
}

void WebServ::stop(void)
{

    //TODO close sockets

    Config::clear();

    cliMap::iterator      cliIt  = _clients.begin();
    cliMap::iterator      cliEnd  = _clients.end();
    while(cliIt != cliEnd)
    {
        Client* tmp = cliIt->second;
        delete tmp;
        cliIt++;
    }
    _clients.clear();
}

bool WebServ::isServerSocket(const int& fd)
{
    listenMap::iterator lsIt        = _listeners.begin();
    listenMap::iterator lsEnd       = _listeners.end();

    while (lsIt != lsEnd)
    {
        if (lsIt->second == fd)
            return(true);
        lsIt++;
    }
    return(false);
}
