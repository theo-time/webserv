/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <fstream>
# include <cstdlib>
# include <string>
# include <vector>
# include <queue>

# include "VirtualServer.hpp"
class VirtualServer;

/* 
Static class initialized after conf file parsing 
*/ 
class Config
{
    public:

        typedef std::queue<std::string>         queue;
        typedef std::vector<VirtualServer*>     srvVect;

        static void                             init(const std::string& filename);
        static bool                             isValid(void);
        static srvVect&                         getVirtualServers(void);
        static void                             clear(void);

        static unsigned int                     getClientMaxBodySize(void);

        const static unsigned int               _clientMaxBodySize_min;
        const static unsigned int               _clientMaxBodySize_max;

    private:

        static bool                             _valid;

        static unsigned int                     _clientMaxBodySize;
        static srvVect                          _virtualServers;

        static std::string                      _tmpConfData;
        static queue                            _tmpVarConf;
        static queue                            _tmpSrvConf;

        static bool                             checkConfFile(const std::string& filename);
        static bool                             parseConfData();
        static void                             addVarConf(std::string& line);
        static void                             addSrvConf(std::string& line);
};

#endif
