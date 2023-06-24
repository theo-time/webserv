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

typedef std::map<int, std::string>      intStrMap;

VirtualServer::VirtualServer(const unsigned int& port, const std::string& root, bool get, bool post, bool del) : 
    _port(port), _root(root), _allowGet(get), _allowPost(post), _allowDel(del)
{
    init();
}

void VirtualServer::init(void)
{
    _host               = "localhost";
    _name               = "";
    _fd                 = -1;
    _index              = "index.html";
    _errorPages[404]    = "./default/404.html";
    _errorPages[500]    = "./default/500.html";
    _clientMaxBodySize  = Config::getClientMaxBodySize();
}

VirtualServer::~VirtualServer(void){}

VirtualServer::VirtualServer(void){}

std::ostream& operator<<(std::ostream& o, VirtualServer& me)
{
    std::cout << "Virtual Server configuration" << std::endl;
    std::cout << "      port                " << me.getPort() << std::endl;
    std::cout << "      host                " << me.getHost() << std::endl;
    std::cout << "      name                " << me.getName() << std::endl;
    std::cout << "      index               " << me.getIndex() << std::endl;
    std::cout << "      root                " << me.getRoot() << std::endl;
    std::cout << "      fd                  " << me.getFd() << std::endl;
    std::cout << "      allowGet            " << me.isGetAllowed() << std::endl;
    std::cout << "      allowPost           " << me.isPostAllowed() << std::endl;
    std::cout << "      allowDel            " << me.isDelAllowed() << std::endl;
    std::cout << "      clientMaxBodySize   " << me.getClientMaxBodySize() << std::endl;

    intStrMap::iterator it = me.getErrorPages().begin();
    while (it != me.getErrorPages().end())
    {
        std::cout << "      " << it-> first << "                 " << it->second << std::endl;
        it++;
    }
    
    return(o);
}

VirtualServer::VirtualServer(const VirtualServer& src)
{
    *this = src;
}

VirtualServer& VirtualServer::operator=(const VirtualServer& rhs)
{
    (void)rhs;
    return(*this);
}

void VirtualServer::setErrorPages(intStrMap conf)
{
    intStrMap::iterator it = conf.begin();
    while (it != conf.end())
    {
        //TODO check code
        _errorPages[it->first] = _root + it->second;
        it++;
    }
}

void VirtualServer::setLocationsConf(strQueue conf)
{
    _tmpLocationsConf = conf;
    
    while (!_tmpLocationsConf.empty())
    {
        std::cout << "      locations:" << _tmpLocationsConf.front() << std::endl;
        _tmpLocationsConf.pop();
    }
}

void VirtualServer::setPort(unsigned int port)
{
    // TODO check valid port number
    _port = port;
}

void VirtualServer::setFd(int fd)
{
    // TODO check valid fd number
    _fd = fd;
}

void VirtualServer::setHost(std::string host)
{
    // TODO check valid host
    _host = host;
}

void VirtualServer::setName(std::string name)
{
    // TODO check valid name
    _name = name;
}

void VirtualServer::setIndex(std::string index)
{
    // TODO check valid index
    _index = index;
}

void VirtualServer::setRoot(std::string root)
{
    // TODO check valid root
    _root = root;
}

void VirtualServer::setClientMaxBodySize(unsigned int value)
{
    // TODO check value
    _clientMaxBodySize = value;
}

std::string VirtualServer::getName(void) const
{
    return(_name);
}

intStrMap& VirtualServer::getErrorPages(void)
{
    return(_errorPages);
}

unsigned int VirtualServer::getPort(void) const
{
    return(_port);
}

int VirtualServer::getFd(void) const
{
    return(_fd);
}

std::string VirtualServer::getHost(void) const
{
    return(_host);
}

std::string VirtualServer::getIndex(void) const
{
    return(_index);
}

std::string VirtualServer::getRoot(void) const
{
    return(_root);
}

bool VirtualServer::isGetAllowed(void) const
{
    return(_allowGet);
}

bool VirtualServer::isPostAllowed(void) const
{
    return(_allowPost);
}

bool VirtualServer::isDelAllowed(void) const
{
    return(_allowDel);
}

unsigned int VirtualServer::getClientMaxBodySize(void) const
{
    return(_clientMaxBodySize);
}
