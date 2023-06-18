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
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/time.h>

# include <iostream>
# include <cstring>
# include <string>
# include <vector>
# include <ctime>
# include <map>

# include "Config.hpp"
# include "Client.hpp"

/* 
Static class representing the engine 
*/
class WebServ
{
    public:

        typedef std::vector<VirtualServer*>     srvVect;
        typedef std::map<int, Client*>          cliMap;
        typedef std::map<int, int>              listenMap;

        static bool                             runListeners(void);
        static void                             stop(void);

    private:    
        static fd_set                           _master_set_recv;
        static fd_set                           _master_set_write;
        static int                              _max_fd;
        static cliMap                           _clients;
        static listenMap                        _listeners;

        static bool                             process(void);
        static bool                             init(void);
        static bool                             acceptNewCnx(const int& fd);
        static bool                             readRequest(const int& fd, Client& c);
        static bool                             sendResponse(const int& fd, Client& c);
        static void                             add(const int& fd, fd_set& set);
        static void                             del(const int& fd, fd_set& set);
        static void                             closeCnx(const int& fd);
        static bool                             isServerSocket(const int& fd);
};

#endif
