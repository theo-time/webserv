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
# include <cstdlib>
# include <fstream>
# include <string>
# include <vector>
# include <queue>
# include <ctime>
# include <cstring>

# include "VirtualServer.hpp"

class WebServ
{
    public:

        typedef std::queue<std::string>         queue;
        typedef std::vector<VirtualServer*>     srvVect;

        static bool                             init(const std::string& filename);
        static bool                             runListeners(void);
        static void                             stop(void);

    private:    

        static unsigned int                     _clientMaxBodySize;
        static srvVect                          _virtualServers;

        static queue                            _tmpVarConf;
        static queue                            _tmpSrvConf;

        static bool                             checkConfFile(const std::string& filename);
        static void                             addVarConf(std::string& line);
        static void                             addSrvConf(std::string& line);
        static bool                             initFdSet(void);
};

#endif
