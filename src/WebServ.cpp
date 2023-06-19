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
            add(serverSocket, _master_set_recv);
            _listeners[(*srvIt)->getPort()] = serverSocket; 
        }
        srvIt++;
    }
    return(true);
}

bool WebServ::process(void)
{
  struct pollfd fds[200];
  char   buffer[1024];
 int rc = 0;
 int nfds = 1;
 int current_size = 0;
 int new_sd = -1;
 int i, len;
  /*************************************************************/
  /* Initialize the pollfd structure                           */
  /*************************************************************/
  memset(fds, 0 , sizeof(fds));

  /*************************************************************/
  /* Set up the initial listening socket                       */
  /*************************************************************/

  fds[0].fd = _listeners.begin()->second;
  fds[0].events = POLLIN;

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
    for (i = 0; i < current_size; i++)
    {
      /*********************************************************/
      /* Loop through to find the descriptors that returned    */
      /* POLLIN and determine whether it's the listening       */
      /* or the active connection.                             */
      /*********************************************************/
      if(fds[i].revents == 0)
        continue;

      /*********************************************************/
      /* If revents is not POLLIN, it's an unexpected result,  */
      /* log and end the server.                               */
      /*********************************************************/
      if(fds[i].revents != POLLIN)
      {
        printf("  Error! revents = %d\n", fds[i].revents);
        return(false);
        break;

      }
      if (fds[i].fd == _listeners.begin()->second)
      {
        /*******************************************************/
        /* Listening descriptor is readable.                   */
        /*******************************************************/
        printf("  Listening socket is readable\n");

        /*******************************************************/
        /* Accept all incoming connections that are            */
        /* queued up on the listening socket before we         */
        /* loop back and call poll again.                      */
        /*******************************************************/
        do
        {
          /*****************************************************/
          /* Accept each incoming connection. If               */
          /* accept fails with EWOULDBLOCK, then we            */
          /* have accepted all of them. Any other              */
          /* failure on accept will cause us to end the        */
          /* server.                                           */
          /*****************************************************/
          new_sd = accept(_listeners.begin()->second, NULL, NULL);
          if (new_sd < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("  accept() failed");
              return(false);
            }
            break;
          }
          /*****************************************************/
          /* Add the new incoming connection to the            */
          /* pollfd structure                                  */
          /*****************************************************/
          printf("  New incoming connection - %d\n", new_sd);
          fds[nfds].fd = new_sd;
          fds[nfds].events = POLLIN;
          nfds++;

          /*****************************************************/
          /* Loop back up and accept another incoming          */
          /* connection                                        */
          /*****************************************************/
        } while (new_sd != -1);
      }

      /*********************************************************/
      /* This is not the listening socket, therefore an        */
      /* existing connection must be readable                  */
      /*********************************************************/

      else
      {
        printf("  Descriptor %d is readable\n", fds[i].fd);
        /*******************************************************/
        /* Receive all incoming data on this socket            */
        /* before we loop back and call poll again.            */
        /*******************************************************/

        do
        {
          /*****************************************************/
          /* Receive data on this connection until the         */
          /* recv fails with EWOULDBLOCK. If any other         */
          /* failure occurs, we will close the                 */
          /* connection.                                       */
          /*****************************************************/
          rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
          if (rc < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("  recv() failed");
              return(false);
            }
            break;
          }

          /*****************************************************/
          /* Check to see if the connection has been           */
          /* closed by the client                              */
          /*****************************************************/
          if (rc == 0)
          {
            printf("  Connection closed\n");
              return(false);
            break;
          }

          /*****************************************************/
          /* Data was received                                 */
          /*****************************************************/
          len = rc;
          printf("  %d bytes received\n", len);

          /*****************************************************/
          /* Echo the data back to the client                  */
          /*****************************************************/
            char response[]  = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
          rc = send(fds[i].fd, response, sizeof(response), 0);
          if (rc < 0)
          {
            perror("  send() failed");
              return(false);
            break;
          }

        } while(true);
      }
    }
 } 

    
/*     while (true)
    {
        working_set_recv = _master_set_recv;
        working_set_write = _master_set_write;
        std::cout << "  Preparing select() " << std::endl;
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
            else if (FD_ISSET(i, &working_set_recv) && _clients.count(i))
                readRequest(i, *_clients[i]);
            else if (FD_ISSET(i, &working_set_write) && _clients.count(i))
                sendResponse(i, *_clients[i]);
        }
    } */
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
        _clients[clientSocket] = new Client(clientSocket, clientAddress);
        std::cout << "  New incoming connection - fd " << clientSocket << std::endl;
    } while (clientSocket != -1);

    return(true);
}

bool WebServ::readRequest(const int &fd, Client &c)
{
    std::cout << "  Reading request - fd " << fd << std::endl;
    char        buffer[1024];
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

bool WebServ::sendResponse(const int &fd, Client &c)
{
    std::cout << "  Sending response - fd " << fd << std::endl;
    char response[]  = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";

    (void)c; 
    int rc  = send(fd, response, sizeof(response), 0);
    if (rc < 0)
    {
        std::cerr << "  send() failed" << std::endl;
        closeCnx(fd);
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
