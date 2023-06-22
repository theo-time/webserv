/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <sys/socket.h>
# include <sys/poll.h>
# include <sys/time.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>

# include <iostream>
# include <cstring>
# include <string>
# include <vector>
# include <ctime>
# include <map>

# include "Config.hpp"
# include "ClientCnx.hpp"

/* 
Static class representing the engine 
*/
class WebServ
{
    public:

        typedef std::vector<VirtualServer*>     srvVect;
        typedef std::map<int, ClientCnx*>       cliMap;
        typedef std::map<ClientCnx*, int>       uriMap;
        typedef std::map<int, int>              listenMap;

        static bool                             runListeners(void);
        static void                             stop(void);
        
        static void                             closeCnx(const int& fd);
        static void                             addToRecvSet(const int& fd, ClientCnx& c);
        static void                             delFromRecvSet(const int& fd, ClientCnx& c);

    private:    
        static fd_set                           _master_set_recv;
        static fd_set                           _master_set_write;
        static int                              _max_fd;
        static cliMap                           _clients;
        static uriMap                           _reqRessources;
        static listenMap                        _listeners;

        static struct pollfd fds[200];

        static bool                             process(void);
        static bool                             init(void);
        static bool                             acceptNewCnx(const int& fd);
        static bool                             readRessource(const int& fd);
        static bool                             readRequest(const int& fd, ClientCnx& c);
        static bool                             sendResponse(const int& fd, ClientCnx& c);
        static void                             add(const int& fd, fd_set& set);
        static void                             del(const int& fd, fd_set& set);
        static bool                             isServerSocket(const int& fd);
        static bool                             isRessourceFd(const int& fd);
};

#endif
