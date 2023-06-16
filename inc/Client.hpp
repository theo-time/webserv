/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <arpa/inet.h>
# include <iostream>
# include <string>

# include "VirtualServer.hpp"

/* 
Contains client socket info, request and response
*/
class Client
{
    public:

        Client(const int& fd, struct sockaddr_in& clientAddress, VirtualServer& server);
        ~Client(void);
        
        Client& operator=(const Client& rhs);
        
        // TODO créer un objet HttpRequest ou un appel CGI selon parsing
        void                newRequest(std::string req);
        
        // TODO renvoyer le contenu de l'objet HttpResponse
        std::string         getResponse(void) const;

    private:

        Client(const Client& src);
        
        int                 _fd;
        struct sockaddr_in  _clientAddress;
        VirtualServer       _server;
        std::string         _response;

        void                setFd(int fd);
        int                 getFd(void) const;
        void                setClientAddress(struct sockaddr_in clientAddress);
        struct sockaddr_in  getClientAddress(void) const;
        void                setServer(VirtualServer& server);
        VirtualServer       getServer(void) const;
};

#endif
