/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUALSERVER_HPP
# define VIRTUALSERVER_HPP

# include <iostream>
# include <string>

/*
Represents 1 server config (One instance per server block in conf file)
_fd contains server socket
*/
class VirtualServer
{
    public:

        VirtualServer(const unsigned int& port);
        VirtualServer(const VirtualServer& src);
        ~VirtualServer(void);
        
        VirtualServer& operator=(const VirtualServer& rhs);

        unsigned int        getPort(void) const;
        int                 getFd(void) const;
        std::string         getHost(void) const;
        std::string         getName(void) const;
        std::string         getIndex(void) const;
        std::string         getRoot(void) const;
        
        void                setFd(int fd);

    private:

        VirtualServer(void);

        void                setPort(unsigned int port);
        void                setHost(std::string host);
        void                setName(std::string name);
        void                setIndex(std::string index);
        void                setRoot(std::string root);

        unsigned int        _port;
        std::string         _host;
        std::string         _name;
        std::string         _index;
        std::string         _root;
        int                 _fd;
};

#endif
