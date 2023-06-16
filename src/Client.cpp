/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Client.hpp"

Client::Client(const Client& src) : 
_fd(src.getFd()), _clientAddress(src.getClientAddress()), _server(src.getServer())
{
    *this = src;
}

Client::Client(const int& fd, struct sockaddr_in& clientAddress, VirtualServer& server) :
_fd(fd), _clientAddress(clientAddress), _server(server) {}

Client::~Client(void){}

Client& Client::operator=(const Client& rhs)
{
    if (this != &rhs)
    {
        _fd = rhs.getFd() ;
        _clientAddress = rhs.getClientAddress();
        _server = rhs.getServer();
    }
    return(*this);
}

std::string Client::getResponse(void) const
{
    return("HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!");
}

void Client::newRequest(std::string req)
{
    std::cout << req << std::endl;
}

void Client::setFd(int fd)
{
    _fd = fd;
}

int Client::getFd(void) const
{
    return(_fd);
}

void Client::setClientAddress(struct sockaddr_in clientAddress)
{
    _clientAddress = clientAddress;
}

struct sockaddr_in Client::getClientAddress(void) const
{
    return(_clientAddress);
}

void Client::setServer(VirtualServer& server)
{
    _server = server;
}

VirtualServer Client::getServer(void) const
{
    return(_server);
}
