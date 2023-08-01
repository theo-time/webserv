/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: teliet <teliet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:03 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/27 17:54:09 by teliet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"

typedef std::map<int, std::string>      intStrMap;
typedef std::vector<Location*>          vectLocation;

VirtualServer::VirtualServer(const unsigned int& port, const std::string& root, bool get, bool post, bool del, const int clientMaxBodySize) : 
    _port(port), _root(root), _allowGet(get), _allowPost(post), _allowDel(del), _clientMaxBodySize(clientMaxBodySize)
{
    init();
}

void VirtualServer::init(void)
{
    _name               = "localhost";
    _fd                 = -1;
    _index              = "index.html";

    _locations.push_back(new Location("_internal", _root, _index, _allowGet, _allowPost, _allowDel, _clientMaxBodySize, _errorPages));
}

VirtualServer::~VirtualServer(void)
{
    while (!_locations.empty())
    {
        delete _locations[0];
        _locations.erase(_locations.begin());
    }
}

VirtualServer::VirtualServer(void){}

std::ostream& operator<<(std::ostream& o, VirtualServer& me)
{
    std::cout << "Virtual Server configuration" << std::endl;
    std::cout << "          port                " << me.getPort() << std::endl;
    std::cout << "          name                " << me.getName() << std::endl;
    std::cout << "          index               " << me.getIndex() << std::endl;
    std::cout << "          root                " << me.getRoot() << std::endl;
    std::cout << "          fd                  " << me.getFd() << std::endl;
    std::cout << "          allowGet            " << me.isGetAllowed() << std::endl;
    std::cout << "          allowPost           " << me.isPostAllowed() << std::endl;
    std::cout << "          allowDel            " << me.isDelAllowed() << std::endl;
    std::cout << "          clientMaxBodySize   " << me.getClientMaxBodySize() << std::endl;

    intStrMap::iterator it = me.getErrorPages().begin();
    while (it != me.getErrorPages().end())
    {
        std::cout << "          " << it-> first << "                 " << it->second << std::endl;
        it++;
    }

    vectLocation::iterator itv = me.getLocations().begin();
    while (itv != me.getLocations().end())
    {
        std::cout << "          " << **itv << std::endl;
        itv++;
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
        _errorPages[it->first] = "." + _root + it->second;
        it++;
    }
}

void VirtualServer::setLocationsConf(strQueue conf)
{
    _tmpLocationsConf = conf;
    
    while (!_tmpLocationsConf.empty())
    {
        std::size_t     sep = _tmpLocationsConf.front().find("{");

        if (sep == std::string::npos)
        {
            std::cout << "Error: invalid location configuration: " << _tmpLocationsConf.front() << std::endl;
            _tmpLocationsConf.pop();
            continue;
        }

        std::string     name = _tmpLocationsConf.front().substr(0, sep);
        std::string     conf = _tmpLocationsConf.front().substr(sep + 1);
        if (name.empty() || conf.empty() || conf.find(";") == std::string::npos)
        {
            std::cout << "Error: invalid location configuration: " << _tmpLocationsConf.front() << std::endl;
            _tmpLocationsConf.pop();
            continue;
        }

        // check each line has key and value
        bool    error = false;
        sep = conf.find(";");
        while (sep != std::string::npos)
        {
            std::size_t key = conf.find("=");
            if (key == std::string::npos || key >= sep - 1)
            {
                error = true;
                break;
            }
            sep = conf.find(";", sep + 1);
        }
        if (error)
        {
            std::cout << "Warning: invalid location configuration ignored: " << _tmpLocationsConf.front() << std::endl;
            _tmpLocationsConf.pop();
            continue;
        }

        _locations.push_back(new Location(name, _root, "", _allowGet, _allowPost, _allowDel, _clientMaxBodySize, _errorPages, conf));
        _tmpLocationsConf.pop();
    }
}

void VirtualServer::setPort(unsigned int port)
{
    _port = port;
}

void VirtualServer::setFd(int fd)
{
    _fd = fd;
}

void VirtualServer::setName(std::string name)
{
    _name = name;
}

void VirtualServer::setIndex(std::string index)
{
    _index = index;
}

void VirtualServer::setRoot(std::string root)
{
    _root = root;
}

void VirtualServer::setClientMaxBodySize(unsigned int value)
{
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

vectLocation& VirtualServer::getLocations(void)
{
    return(_locations);
}

unsigned int VirtualServer::getPort(void) const
{
    return(_port);
}

int VirtualServer::getFd(void) const
{
    return(_fd);
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
