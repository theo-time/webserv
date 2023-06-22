/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientCnx.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ClientCnx.hpp"

ClientCnx::ClientCnx(const ClientCnx& src) : 
_fd(src.getFd()), _clientAddress(src.getClientAddress()), _server(src.getServer())
{
    *this = src;
}

ClientCnx::ClientCnx(const int& fd, struct sockaddr_in& clientAddress) :
_fd(fd), _clientAddress(clientAddress) {}

ClientCnx::~ClientCnx(void){}

ClientCnx& ClientCnx::operator=(const ClientCnx& rhs)
{
    if (this != &rhs)
    {
        _fd = rhs.getFd() ;
        _clientAddress = rhs.getClientAddress();
        _server = rhs.getServer();
    }
    return(*this);
}

std::string ClientCnx::getResponse(void) const
{
    return("HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!");
}

void ClientCnx::newRequest(std::string req)
{
    std::cout << req << std::endl;
}

void ClientCnx::setFd(int fd)
{
    _fd = fd;
}

int ClientCnx::getFd(void) const
{
    return(_fd);
}

void ClientCnx::setClientAddress(struct sockaddr_in clientAddress)
{
    _clientAddress = clientAddress;
}

struct sockaddr_in ClientCnx::getClientAddress(void) const
{
    return(_clientAddress);
}

void ClientCnx::setServer(VirtualServer& server)
{
    _server = server;
}

VirtualServer ClientCnx::getServer(void) const
{
    return(_server);
}
