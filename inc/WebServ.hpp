/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jde-la-f <jde-la-f@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/29 18:27:29 by jde-la-f         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define BUFFER_SIZE 1024

# include <unistd.h>
# include <fcntl.h>

# include <iostream>
# include <cstring>
# include <string>
# include <vector>
# include <ctime>
# include <map>

# include "Config.hpp"

class Request;

/* 
Static class representing the engine 
*/
class WebServ
{
    public:

        typedef std::vector<VirtualServer*>         srvVect;
        typedef std::map<int, Request*>             intCliMap;
        typedef std::map<int, int>                  intMap;

        static bool                                 runListeners(void);
        static void                                 stop(void);
	    static const std::string                    httpMethods[9];
        static void                                 addFd2Select(const int& fd);
        static void                                 delFd2Select(const int& fd);

    private:

        static fd_set                               _master_set_recv;
        static fd_set                               _master_set_write;
        static int                                  _max_fd;
        static intMap                               _listeners;
        static intCliMap                            _requests;

        static bool                                 init(void);
        static bool                                 process(void);
        static bool                                 runListener(VirtualServer* srv);
        static void                                 prepSelect(void);
        static bool                                 acceptNewCnx(const int& fd);
        static bool                                 readRequest(const int& fd, Request& c);
        static bool                                 sendResponse(const int& fd, Request& c);
        static bool                                 isServerSocket(const int& fd);
        static void                                 add(const int& fd, fd_set& set);
        static void                                 del(const int& fd, fd_set& set);
        static void                                 closeCnx(const int& fd);
        static bool                                 userExit(void);
        static void                                 handleTimeout(void);
};

#endif
