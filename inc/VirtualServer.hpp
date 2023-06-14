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
        void                setName(unsigned int name);
        std::string         getName(void);
        void                setIndex(unsigned int index);
        std::string         getIndex(void);
        void                setRoot(unsigned int root);
        std::string         getRoot(void);

    private:
        VirtualServer(void);

        unsigned int        _port;
        std::string         _name;
        std::string         _index;
        std::string         _root;
};

#endif
