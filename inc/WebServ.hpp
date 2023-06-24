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
# include "ClientCnx.hpp"
class ClientCnx;

/* 
Static class representing the engine 
*/
class WebServ
{
    public:

        typedef std::vector<VirtualServer*>         srvVect;
        typedef std::map<int, ClientCnx*>           intCliMap;
        typedef std::map<ClientCnx*, std::string>   cliStrMap;
        typedef std::map<int, std::string>          intStrMap;
        typedef std::map<int, int>                  intMap;

        static bool                                 runListeners(void);
        static void                                 getRessource(const std::string& path, ClientCnx& c);
        static void                                 stop(void);

    private:

        static fd_set                               _master_set_recv;
        static fd_set                               _master_set_write;
        static int                                  _max_fd;
        static intMap                               _listeners;
        static intCliMap                            _clients;
        static cliStrMap                            _reqRessources;
        static intStrMap                            _fdRessources;

        static bool                                 init(void);
        static bool                                 process(void);
        static bool                                 acceptNewCnx(const int& fd);
        static bool                                 readRessource(const int& fd);
        static bool                                 readRequest(const int& fd, ClientCnx& c);
        static bool                                 sendResponse(const int& fd, ClientCnx& c);
        static bool                                 isServerSocket(const int& fd);
        static bool                                 isListedRessource(const std::string& path);
        static void                                 add(const int& fd, fd_set& set);
        static void                                 del(const int& fd, fd_set& set);
        static void                                 closeCnx(const int& fd);
};

#endif
