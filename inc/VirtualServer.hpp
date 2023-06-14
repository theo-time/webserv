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

class VirtualServer
{
    public:
        VirtualServer(const unsigned int& port);
        VirtualServer(const VirtualServer& src);
        ~VirtualServer(void);
        
        VirtualServer& operator=(const VirtualServer& rhs);

        void                setPort(unsigned int port);
        unsigned int        getPort(void);
        void                setFd(int fd);
        int                 getFd(void);
        void                setName(std::string name);
        std::string         getName(void);
        void                setIndex(std::string index);
        std::string         getIndex(void);
        void                setRoot(std::string root);
        std::string         getRoot(void);

    private:
        VirtualServer(void);

        unsigned int        _port;
        std::string         _name;
        std::string         _index;
        std::string         _root;
        int                 _fd;
};

#endif
