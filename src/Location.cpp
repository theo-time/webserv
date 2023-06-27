/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

typedef std::queue<std::string>         strQueue;
typedef std::map<int, std::string>      intStrMap;

Location::Location(const std::string& name, const std::string& conf) :
_name(name), _type("std"), _autoIndex(false), _allowGet(false), _allowPost(false), _allowDel(false), _clientMaxBodySize(Config::getClientMaxBodySize())
{
    std::cout << "new Location " << _name << ": " << conf << std::endl;
    
    std::string     tmpLine = conf;
    strQueue        tmpVars;
    std::size_t     sep;

    while (!tmpLine.empty())
    {
        sep = tmpLine.find(';');
        tmpVars.push(tmpLine.substr(0, sep));
        tmpLine.erase(0, sep + 1);
    }

    if (tmpVars.size() == 1 && tmpVars.front().substr(0, 7) == "return=")
    {
        _type = "http";
        _path = tmpVars.front().substr(7, tmpVars.front().length() - 7);
        std::cout << "key: return value:" << _path << std::endl;
        return;
    }

    if (tmpVars.front().substr(0, 4) == "cgi_")
    {
        _type = "cgi";
        while (!tmpVars.empty())
        {
            sep = tmpVars.front().find('=');
            std::string key = tmpVars.front().substr(0, sep);
            std::string valueStr = tmpVars.front().substr(sep + 1, tmpVars.front().length() - sep - 1);

            if (key == "cgi_root" )
            {
                _root = valueStr;
                std::cout << "key: " << key << " value:" << valueStr << std::endl;
                tmpVars.pop();
                continue;
            }

            if (key == "cgi_path" )
            {
                _path = valueStr;
                std::cout << "key: " << key << " value:" << valueStr << std::endl;
                tmpVars.pop();
                continue;
            }

            if (key == "cgi_pgname" )
            {
                _pgname = valueStr;
                std::cout << "key: " << key << " value:" << valueStr << std::endl;
                tmpVars.pop();
                continue;
            }
            
            sep = key.find("cgi_error_page:");
            if(sep != std::string::npos)
            {
                std::stringstream   ss;
                int                 code;
                ss << key.substr(15, key.length());
                ss >> code;
                _errorPages[code] = valueStr; // TODO check code
                std::cout << "key: " << code << " value:" << valueStr << std::endl;
                tmpVars.pop();
                continue;
            }

            if(key == "cgi_methods")
            {
                while (!valueStr.empty())
                {
                    std::string tmp = "";
                    sep = valueStr.find(",");
                    if (sep == std::string::npos)
                    {
                        tmp = valueStr;
                        valueStr.clear();
                    }
                    else
                    {
                        tmp = valueStr.substr(0, sep);
                        valueStr.erase(0, sep + 1);
                    }
                    if (tmp == "GET")
                        _allowGet = true;
                    else if (tmp == "POST")
                        _allowPost = true;
                    else if (tmp == "DELETE")
                        _allowDel = true;
                }
                tmpVars.pop();
                continue;
            }
            std::cout << "      Warning: unknown key: " << key << " value:" << valueStr << std::endl;
            tmpVars.pop();
        }

        return;
    }

    while (!tmpVars.empty())
    {
        sep = tmpVars.front().find('=');
        std::string key = tmpVars.front().substr(0, sep);
        std::string valueStr = tmpVars.front().substr(sep + 1, tmpVars.front().length() - sep - 1);

        if (key == "root" )
        {
            _root = valueStr;
            std::cout << "key: " << key << " value:" << valueStr << std::endl;
            tmpVars.pop();
            continue;
        }

        if (key == "index" )
        {
            _index = valueStr;
            std::cout << "key: " << key << " value:" << valueStr << std::endl;
            tmpVars.pop();
            continue;
        }

        std::cout << "      Warning: unknown key: " << key << " value:" << valueStr << std::endl;
        tmpVars.pop();
    }


/*

    

    while (!tmpVars.empty())
    {
        sep = tmpVars.front().find('=');
        std::string key = tmpVars.front().substr(0, sep);
        std::string valueStr = tmpVars.front().substr(sep + 1, tmpVars.front().length() - sep - 1);

        if(key == "listen")
        {
            char*               endPtr      = NULL;
            unsigned long       tmpValue = strtoul(valueStr.c_str(), &endPtr, 0);
            if  (endPtr == valueStr || endPtr != &(valueStr[valueStr.size()]))
            {
                std::cout << "Error: invalid input: " << tmpVars.front() << std::endl;
                return;
            }
            portStr = valueStr;
            port = static_cast<unsigned int>(tmpValue);
            tmpVars.pop();
            continue;
        }

        if(key == "host")
        {
            host = valueStr;
            tmpVars.pop();
            continue;
        }

        if(key == "server_name")
        {
            serverName = valueStr;
            tmpVars.pop();
            continue;
        }

        if(key == "index")
        {
            index = valueStr;
            tmpVars.pop();
            continue;
        }

        if(key == "root")
        {
            root = valueStr;
            tmpVars.pop();
            continue;
        }

        if(key == "methods")
        {
            while (!valueStr.empty())
            {
                std::string tmp = "";
                sep = valueStr.find(",");
                if (sep == std::string::npos)
                {
                    tmp = valueStr;
                    valueStr.clear();
                }
                else
                {
                    tmp = valueStr.substr(0, sep);
                    valueStr.erase(0, sep + 1);
                }
                if (tmp == "GET")
                    isGetAllowed = true;
                else if (tmp == "POST")
                    isPostAllowed = true;
                else if (tmp == "DELETE")
                    isDelAllowed = true;
            }
            tmpVars.pop();
            continue;
        }

        sep = key.find("error_page:");
        if(sep != std::string::npos)
        {
            std::stringstream   ss;
            int                 code;
            ss << key.substr(11, key.length());
            ss >> code;
            tmpErrorPages[code] = valueStr;
            tmpVars.pop();
            continue;
        }

        if(key == "client_max_body_size")
        {
            char*               endPtr      = NULL;
            unsigned long       tmpValue = strtoul(valueStr.c_str(), &endPtr, 0);
            if  (endPtr == valueStr || endPtr != &(valueStr[valueStr.size()]))
            {
                std::cout << "Error: invalid input: " << tmpVars.front() << std::endl;
                return;
            }
            maxBodySizeStr = valueStr;
            maxBodySize = static_cast<unsigned int>(tmpValue);
            tmpVars.pop();
            continue;
        }
        
        std::cout << "      Warning: unknown key: " << key << " value:" << valueStr << std::endl;
        tmpVars.pop();
    }

    if (portStr.empty())
    {
        std::cout << "      Error: missing port number" << std::endl;
        return;
    }

    VirtualServer* tmp = new VirtualServer(port, root, isGetAllowed, isPostAllowed, isDelAllowed);
    if (!tmpLocations.empty())
        tmp->setLocationsConf(tmpLocations);
    if (!host.empty())
        tmp->setHost(host);
    if (!tmpErrorPages.empty())
        tmp->setErrorPages(tmpErrorPages);
    if (!maxBodySizeStr.empty())
        tmp->setClientMaxBodySize(maxBodySize);
    if (!serverName.empty())
    {
        tmp->setName(serverName);
        std::string alias = serverName + ":" + portStr;
        _serverNames[alias] = tmp;
    }
    _virtualServers.push_back(tmp);
    std::cout << "  " << *tmp  << std::endl;
*/    
}

Location::Location(void){}

Location::Location(const Location& src)
{
    *this = src;
}

Location::~Location(void){}

Location& Location::operator=(const Location& rhs)
{
    (void)rhs;
    return(*this);
}

std::string Location::getName(void) const
{
    return(_name);
}

std::string Location::getType(void) const
{
    return(_type);
}

std::string Location::getRoot(void) const
{
    return(_root);
}

std::string Location::getIndex(void) const
{
    return(_index);
}

std::string Location::getPath(void) const
{
    return(_path);
}

std::string Location::getPgName(void) const
{
    return(_pgname);
}

unsigned int Location::getClientMaxBodySize(void) const
{
    return(_clientMaxBodySize);
}

intStrMap& Location::getErrorPages(void)
{
    return(_errorPages);
}

bool Location::isGetAllowed(void) const
{
    return(_allowGet);
}

bool Location::isPostAllowed(void) const
{
    return(_allowPost);
}

bool Location::isDelAllowed(void) const
{
    return(_allowDel);
}

bool Location::isAutoIndex(void) const
{
    return(_autoIndex);
}

std::ostream& operator<<(std::ostream& o, Location& me)
{
    std::cout << "Location " << me.getName() << std::endl;
    return(o);
}
