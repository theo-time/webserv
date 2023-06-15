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

unsigned int                                WebServ::_clientMaxBodySize = 1024 * 1024;

typedef std::queue<std::string>             queue;
typedef std::vector<VirtualServer*>         srvVect;

srvVect                                     WebServ::_virtualServers;
queue                                       WebServ::_tmpVarConf;
queue                                       WebServ::_tmpSrvConf;

bool WebServ::init(const std::string& filename)
{
    if (!checkConfFile(filename))
        return(false);
        
    if (_tmpSrvConf.empty())
    {
        std::cout << "Error: missing virtual server configuration" << std::endl;
        return(false);
    }

    while (!_tmpVarConf.empty())
    {
        addVarConf(_tmpVarConf.front());
        _tmpVarConf.pop();
    }

    std::cout << "Found " << _tmpSrvConf.size() << " virtual server configuration(s)" << std::endl;
    while (!_tmpSrvConf.empty())
    {
        addSrvConf(_tmpSrvConf.front());
        _tmpSrvConf.pop();
    }

    return(true);
}

bool WebServ::runListeners(void)
{
    if (_virtualServers.empty())
    {
        std::cout << "Error: no configured virtual server" << std::endl;
        return(false);
    }

    // TODO boucler pour chaque host:port

    // Create an AF_INET6 stream socket to receive incoming connections on
    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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
    serverAddress.sin_port = htons(_virtualServers.at(0)->getPort()); 
    rc = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (rc < 0)
    {
        std::cerr << "Error : socket binding failed" << std::endl;
        close(serverSocket);
        return(false);
    }

    // Set the listen back log
    rc = listen(serverSocket, 10);
    if (rc < 0)
    {
        std::cerr << "Error : socket listen failed" << std::endl;
        close(serverSocket);
        return(false);
    }
    _virtualServers.at(0)->setFd(serverSocket);
    std::cout << "Waiting for connexion: port " << _virtualServers.at(0)->getPort() <<"..." << std::endl;

    return(initFdSet());
}

bool WebServ::initFdSet(void)
{
    int                     i, rc, len = 1;
    int                     desc_ready, close_conn, end_server = 0;
    int                     max_sd, new_sd;
    fd_set                  master_set, working_set; 
    struct timeval          timeout;
    char                    buffer[1024];
    
    /*************************************************************/
    /* Initialize the master fd_set                              */
    /*************************************************************/
    FD_ZERO(&master_set);
    max_sd = _virtualServers.at(0)->getFd(); // TODO boucler sur _virtualServers
    FD_SET(_virtualServers.at(0)->getFd(), &master_set);

    /*************************************************************/
    /* Initialize the timeval struct to 3 minutes.  If no        */
    /* activity after 3 minutes this program will end.           */
    /*************************************************************/
    timeout.tv_sec  = 3 * 60; // TODO ajuster timer
    timeout.tv_usec = 0;

    /*************************************************************/
    /* Loop waiting for incoming connects or for incoming data   */
    /* on any of the connected sockets.                          */
    /*************************************************************/
    while (end_server == 0)
    {
        /**********************************************************/
        /* Copy the master fd_set over to the working fd_set.     */
        /**********************************************************/
        memcpy(&working_set, &master_set, sizeof(master_set));

        /**********************************************************/
        /* Call select() and wait 3 minutes for it to complete.   */
        /**********************************************************/
        printf("Waiting on select()...\n");
        rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

        /**********************************************************/
        /* Check to see if the select call failed.                */
        /**********************************************************/
        if (rc < 0)
        {
            perror("  select() failed");
            break;
        }

        /**********************************************************/
        /* Check to see if the 3 minute time out expired.         */
        /**********************************************************/
        if (rc == 0)
        {
            printf("  select() timed out.  End program.\n");
            break;
        }

        /**********************************************************/
        /* One or more descriptors are readable.  Need to         */
        /* determine which ones they are.                         */
        /**********************************************************/
        desc_ready = rc;
        for (i=0; i <= max_sd  &&  desc_ready > 0; ++i)
        {
            /*******************************************************/
            /* Check to see if this descriptor is ready            */
            /*******************************************************/
            if (FD_ISSET(i, &working_set))
            {
            /****************************************************/
            /* A descriptor was found that was readable - one   */
            /* less has to be looked for.  This is being done   */
            /* so that we can stop looking at the working set   */
            /* once we have found all of the descriptors that   */
            /* were ready.                                      */
            /****************************************************/
            desc_ready -= 1;

            /****************************************************/
            /* Check to see if this is the listening socket     */
            /****************************************************/
            if (i == _virtualServers.at(0)->getFd())
            {
                printf("  Listening socket is readable\n");
                /*************************************************/
                /* Accept all incoming connections that are      */
                /* queued up on the listening socket before we   */
                /* loop back and call select again.              */
                /*************************************************/
                do
                {
                    /**********************************************/
                    /* Accept each incoming connection.  If       */
                    /* accept fails with EWOULDBLOCK, then we     */
                    /* have accepted all of them.  Any other      */
                    /* failure on accept will cause us to end the */
                    /* server.                                    */
                    /**********************************************/
                    new_sd = accept(_virtualServers.at(0)->getFd(), NULL, NULL);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                        perror("  accept() failed");
                        end_server = 1;
                        }
                        break;
                    }

                    /**********************************************/
                    /* Add the new incoming connection to the     */
                    /* master read set                            */
                    /**********************************************/
                    printf("  New incoming connection - %d\n", new_sd);
                    FD_SET(new_sd, &master_set);
                    if (new_sd > max_sd)
                        max_sd = new_sd;

                    /**********************************************/
                    /* Loop back up and accept another incoming   */
                    /* connection                                 */
                    /**********************************************/
                } while (new_sd != -1);
            }

            /****************************************************/
            /* This is not the listening socket, therefore an   */
            /* existing connection must be readable             */
            /****************************************************/
            else
            {
                printf("  Descriptor %d is readable\n", i);
                close_conn = 0;
                /*************************************************/
                /* Receive all incoming data on this socket      */
                /* before we loop back and call select again.    */
                /*************************************************/
                while (1)
                {
                    /**********************************************/
                    /* Receive data on this connection until the  */
                    /* recv fails with EWOULDBLOCK.  If any other */
                    /* failure occurs, we will close the          */
                    /* connection.                                */
                    /**********************************************/
                    rc = recv(i, buffer, sizeof(buffer), 0);
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                        perror("  recv() failed");
                        close_conn = 1;
                        }
                        break;
                    }

                    /**********************************************/
                    /* Check to see if the connection has been    */
                    /* closed by the client                       */
                    /**********************************************/
                    if (rc == 0)
                    {
                        printf("  Connection closed\n");
                        close_conn = 1;
                        break;
                    }

                    /**********************************************/
                    /* Data was received                          */
                    /**********************************************/
                    len = rc;
                    printf("  %d bytes received\n***************\n%s", len, buffer);

                    /**********************************************/
                    /* Echo the data back to the client           */
                    /**********************************************/
                    rc = send(i, buffer, len, 0); // TODO push la requete dans une queue?
                    if (rc < 0)
                    {
                        perror("  send() failed");
                        close_conn = 1;
                        break;
                    }

                }

                /*************************************************/
                /* If the close_conn flag was turned on, we need */
                /* to clean up this active connection.  This     */
                /* clean up process includes removing the        */
                /* descriptor from the master set and            */
                /* determining the new maximum descriptor value  */
                /* based on the bits that are still turned on in */
                /* the master set.                               */
                /*************************************************/
                if (close_conn)
                {
                    close(i);
                    FD_CLR(i, &master_set);
                    if (i == max_sd)
                    {
                        while (FD_ISSET(max_sd, &master_set) == 0)
                        max_sd -= 1;
                    }
                }
            } /* End of existing connection is readable */
            } /* End of if (FD_ISSET(i, &working_set)) */
        } /* End of loop through selectable descriptors */

    } 

    /*************************************************************/
    /* Clean up all of the sockets that are open                 */
    /*************************************************************/
    for (i=0; i <= max_sd; ++i)
    {
        if (FD_ISSET(i, &master_set))
            close(i);
    }
    return(true);
}

bool WebServ::checkConfFile(const std::string& filename)
{
    if (filename.size() < 6  || filename.find(".") == std::string::npos || filename.find(".conf") != filename.size() - 5)
    {
        std::cout << "Error: invalid filename: " << filename << std::endl;
        return(false);
    }

    std::ifstream                           ifs(filename.c_str());
    if (ifs)
    {
        std::string                         line;
        std::string                         srvConf;
        bool                                srv = false; 

        std::cout << "Parsing config file: " << filename << std::endl;
        while(std::getline(ifs, line))
        {
            if (!line.empty())
            {
                // clear comments
                std::size_t found = line.find('#');
                if (found != std::string::npos)
                    line.erase(line.begin() + found, line.end());

                // trim spaces
                while (isspace(line[0]))
                    line.erase(0, 1);
                if (line.empty())
                    continue;
                while (isspace(line[line.length() - 1]))
                    line.erase(line.length() - 1, 1);
                if (line.empty())
                    continue;

                if (srv)
                {
                    found = line.find('}');
                    if (found != std::string::npos)
                    {
                        line.erase(line.begin() + found, line.end());
                        srv = false;
                    }
                    srvConf = srvConf + line;
                    if (!srv)
                    {
                        _tmpSrvConf.push(srvConf);
                        srvConf.clear();
                    }
                }
                else
                {
                    found = line.find("server{");
                    if (found == 0)
                        srv = true;
                    else
                        _tmpVarConf.push(line);
                }
            }
        }
        if (!srv)
            return(true);
        else
        {
            std::cout << "Error: missing closing brace for virtual server configuration: " << srvConf << std::endl;
            return(false);
        }
    }
    else
    {
        std::cout << "Error: could not open file: " << filename << std::endl;
        return(false);
    }
}

void WebServ::addVarConf(std::string& line)
{
    std::size_t sep = line.find('=');
    std::size_t end = line.find(';');
    if (sep == std::string::npos || end == std::string::npos || sep == 0 || sep + 1 == end)
    {
        std::cout << "Error: invalid input: " << line << std::endl;
        return;
    }
    std::string key = line.substr(0, sep);
    std::string valueStr = line.substr(sep + 1, line.length() - sep - 2);

    if(key == "client_max_body_size")
    {
        char*               endPtr      = NULL;
        unsigned long       tmpValue = strtoul(valueStr.c_str(), &endPtr, 0);
        if  (endPtr == valueStr || endPtr != &(valueStr[valueStr.size()]) 
        || tmpValue > 5 * 1024 * 1024 || tmpValue < 1024 * 1024)
        {
            std::cout << "Error: invalid input: " << line << std::endl;
            return;
        }
        _clientMaxBodySize = static_cast<unsigned int>(tmpValue);
        std::cout << "  adding key = " << key;
        std::cout << ", value = " << _clientMaxBodySize << std::endl;
        return;
    }

    std::cout << "  Warning: unknown key: " << line << std::endl;
}

void WebServ::addSrvConf(std::string& line)
{
    std::cout << "  parsing Server..." << std::endl;
    
    queue           tmpVars;
    std::string     tmpLine = line;
    std::size_t     sep = tmpLine.find(';');
    while (sep != std::string::npos)
    {
        tmpVars.push(tmpLine.substr(0, sep));
        tmpLine.erase(0, sep + 1);
        sep = tmpLine.find(';');
    }

    std::string     portStr;
    unsigned int    port;
    std::string     index, root;
    while (!tmpVars.empty())
    {
        sep = tmpVars.front().find('=');
        std::string key = tmpVars.front().substr(0, sep);
        std::string valueStr = tmpVars.front().substr(sep + 1, tmpVars.front().length() - sep - 1);

        if(key == "listen")
        {
            char*               endPtr      = NULL;
            unsigned long       tmpValue = strtoul(valueStr.c_str(), &endPtr, 0);
            if  (endPtr == valueStr || endPtr != &(valueStr[valueStr.size()]))
            {
                std::cout << "Error: invalid input: " << tmpVars.front() << std::endl;
                return;
            }
            portStr = valueStr;
            port = static_cast<unsigned int>(tmpValue);
            std::cout << "      key = " << key;
            std::cout << ", value = " << port << std::endl;
            tmpVars.pop();
            continue;
        }

        if(key == "index")
        {
            index = valueStr;
            std::cout << "      key = " << key;
            std::cout << ", value = " << index << std::endl;
            tmpVars.pop();
            continue;
        }

        if(key == "root")
        {
            root = valueStr;
            std::cout << "      key = " << key;
            std::cout << ", value = " << root << std::endl;
            tmpVars.pop();
            continue;
        }

        std::cout << "      Warning: unknown key: " << tmpVars.front() << std::endl;
        tmpVars.pop();
    }

    if (portStr.empty())
    {
        std::cout << "      Error: missing port number" << std::endl;
        return;
    }

    VirtualServer* tmp = new VirtualServer(port);
    _virtualServers.push_back(tmp);
}

void WebServ::stop(void)
{
    srvVect::iterator      it  = _virtualServers.begin();
    while(it != _virtualServers.end())
    {
        VirtualServer* tmp = *it;
        std::cout << "Deleting virtual server " << tmp->getPort() << std::endl;
        delete tmp;
        it++;
    }
    _virtualServers.clear();
}