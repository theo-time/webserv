/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: teliet <teliet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/27 17:53:10 by teliet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP

# include <iostream>
# include <string>
# include <vector>
# include <queue>
# include <map>

# include "Config.hpp"
# include "Location.hpp"

class Location;

/*
Represents 1 server config (One instance per server block in conf file)
_fd contains server socket
*/
class VirtualServer
{
    public:
        typedef std::vector<Location*>          vectLocation;
        typedef std::queue<std::string>         strQueue;
        typedef std::map<int, std::string>      intStrMap;

        VirtualServer(void);
        VirtualServer(const unsigned int& port, const std::string& root, bool get, bool post, bool del, const int clientMaxBodySize);
        VirtualServer(const VirtualServer& src);
        ~VirtualServer(void);
        
        VirtualServer& operator=(const VirtualServer& rhs);

        unsigned int        getPort(void) const;
        int                 getFd(void) const;
        std::string         getName(void) const;
        std::string         getIndex(void) const;
        std::string         getRoot(void) const;
        std::string         getListen(void) const;
        unsigned int        getClientMaxBodySize(void) const;
        intStrMap&          getErrorPages(void);
        vectLocation&       getLocations(void);
        

        bool                isGetAllowed(void) const;
        bool                isPostAllowed(void) const;
        bool                isDelAllowed(void) const;
        
        void                setFd(int fd);
        void                setLocationsConf(strQueue conf);
        void                setErrorPages(intStrMap conf);
        void                setName(std::string name);
        void                setIndex(std::string index);
        void                setClientMaxBodySize(unsigned int value);
        void                setListen(std::string listen);

    private:

        void                init(void);
        void                setPort(unsigned int port);
        void                setRoot(std::string root);

        unsigned int        _port;
        std::string         _listen;
        std::string         _name;
        std::string         _index;
        std::string         _root;
        int                 _fd;
        intStrMap           _errorPages;
        vectLocation        _locations;
        strQueue            _tmpLocationsConf;
        bool                _allowGet;
        bool                _allowPost;
        bool                _allowDel;
        unsigned int        _clientMaxBodySize;
};

std::ostream& operator<<(std::ostream& o, VirtualServer& me);

#endif
