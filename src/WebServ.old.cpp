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

bool WebServ::runListeners(void)
{
    if (Config::getVirtualServers().empty())
    {
        std::cout << "Error: no configured virtual server" << std::endl;
        return(false);
    }

    // TODO boucler pour chaque VirtualServer, créer un seul socket par host:port

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
    serverAddress.sin_port = htons(Config::getVirtualServers().at(0)->getPort()); 
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
    Config::getVirtualServers().at(0)->setFd(serverSocket);
    std::cout << "Waiting for connexion: port " << Config::getVirtualServers().at(0)->getPort() <<"..." << std::endl;

    return(process());
}

bool WebServ::process(void)
{
    int                     i, rc, len = 1;
    int                     desc_ready, close_conn, end_server = 0;
    int                     max_sd, new_sd;
    fd_set                  master_set, working_set; 
    struct timeval          timeout;
    char                    buffer[1024];
    
    char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
    
    /*************************************************************/
    /* Initialize the master fd_set                              */
    /*************************************************************/
    FD_ZERO(&master_set);
    max_sd = Config::getVirtualServers().at(0)->getFd(); // TODO boucler sur Config::getVirtualServers()
    FD_SET(Config::getVirtualServers().at(0)->getFd(), &master_set);

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
            if (i == Config::getVirtualServers().at(0)->getFd())
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
                    new_sd = accept(Config::getVirtualServers().at(0)->getFd(), NULL, NULL);
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
                    // rc = send(i, buffer, len, 0); // TODO push la requete dans une queue?
                    rc = send(i, response, sizeof(response), 0);
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

void WebServ::stop(void)
{
    srvVect::iterator      it  = Config::getVirtualServers().begin();
    while(it != Config::getVirtualServers().end())
    {
        VirtualServer* tmp = *it;
        std::cout << "Deleting virtual server " << tmp->getPort() << std::endl;
        delete tmp;
        it++;
    }
    Config::getVirtualServers().clear();
}
