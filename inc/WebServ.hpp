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
        typedef std::map<int, Request*>           intCliMap;
        typedef std::map<Request*, std::string>   cliStrMap;
        typedef std::map<int, std::string>          intStrMap;
        typedef std::map<int, int>                  intMap;

        static bool                                 runListeners(void);
        static void                                 getRessource(const std::string& path, Request& c);
        static void                                 stop(void);
        static void                                 add(const int& fd, fd_set& set);
        static fd_set                               & getMasterSetWrite();
        static void                                 addCGIResponseToQueue(Request *request);

    private:

        static fd_set                               _master_set_recv;
        static fd_set                               _master_set_write;
        static int                                  _max_fd;
        static intMap                               _listeners;
        static intCliMap                            _requests;
        static cliStrMap                            _reqRessources;
        static intStrMap                            _fdRessources;

        static bool                                 init(void);
        static bool                                 process(void);
        static bool                                 acceptNewCnx(const int& fd);
        static bool                                 readRessource(const int& fd);
        static bool                                 readRequest(const int& fd, Request& c);
        static bool                                 sendResponse(const int& fd, Request& c);
        static bool                                 isServerSocket(const int& fd);
        static bool                                 isListedRessource(const std::string& path);
        static void                                 del(const int& fd, fd_set& set);
        static void                                 closeCnx(const int& fd);
        static void                                 getRequestConfig(Request& c);
};

#endif
