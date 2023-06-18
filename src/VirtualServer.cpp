/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:03 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:05 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"

VirtualServer::VirtualServer(const unsigned int& port, const std::string& root) : 
    _port(port), _host("localhost"), _name("www.default.com"), _index("index.html"), _root(root)
{}

VirtualServer::~VirtualServer(void){}

VirtualServer::VirtualServer(void){}

VirtualServer::VirtualServer(const VirtualServer& src)
{
    *this = src;
}

VirtualServer& VirtualServer::operator=(const VirtualServer& rhs)
{
    (void)rhs;
    return(*this);
}

void VirtualServer::setPort(unsigned int port)
{
    // TODO check valid port number
    _port = port;
}

unsigned int VirtualServer::getPort(void) const
{
    return(_port);
}

void VirtualServer::setFd(int fd)
{
    // TODO check valid fd number
    _fd = fd;
}

int VirtualServer::getFd(void) const
{
    return(_fd);
}

void VirtualServer::setHost(std::string host)
{
    // TODO check valid host
    _host = host;
}

std::string VirtualServer::getHost(void) const
{
    return(_host);
}

void VirtualServer::setName(std::string name)
{
    // TODO check valid name
    _name = name;
}

std::string VirtualServer::getName(void) const
{
    return(_name);
}

void VirtualServer::setIndex(std::string index)
{
    // TODO check valid index
    _index = index;
}

std::string VirtualServer::getIndex(void) const
{
    return(_index);
}

void VirtualServer::setRoot(std::string root)
{
    // TODO check valid root
    _root = root;
}

std::string VirtualServer::getRoot(void) const
{
    return(_root);
}
