/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jde-la-f <jde-la-f@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/07/31 19:33:01 by jde-la-f         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <fstream>
# include <sstream>
# include <cstdlib>
# include <string>
# include <vector>
# include <queue>
# include <map>

class VirtualServer;

/* 
Static class initialized after conf file parsing 
*/ 
class Config
{
    public:

        typedef std::queue<std::string>                 strQueue;
        typedef std::map<int, std::string>              intStrMap;
        typedef std::map<std::string, VirtualServer*>   srvMap;
        typedef std::vector<VirtualServer*>             srvVect;

        static void                                     init(const std::string& filename);
        static bool&                                    isValid(void);
        static srvVect&                                 getVirtualServers(void);
        static srvMap&                                  getHostsMap(void);
        static void                                     clear(void);

        static unsigned int&                            getClientMaxBodySize(void);

        const static unsigned int                       _clientMaxBodySize_min;
        const static unsigned int                       _clientMaxBodySize_max;
        const static unsigned long                      requestTimeout;

    private:

        static bool                                     _valid;
        
        static unsigned int                             _clientMaxBodySize;
        static srvVect                                  _virtualServers;
        static srvMap                                   _serverNames;
        
        static std::string                              _tmpConfData;
        static strQueue                                 _tmpVarConf;
        static strQueue                                 _tmpSrvConf;
        
        static bool                                     checkConfFile(const std::string& filename);
        static bool                                     parseConfData();
        static bool                                     addVarConf(std::string& line);
        static bool                                     addSrvConf(std::string& line, int i);
};

#endif
