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

Location::Location(const std::string& name, const std::string& root, const std::string& index, 
            const bool allowGet, const bool allowPost, const bool allowDel, 
            const unsigned int clientMaxBodySize, const intStrMap errorPages) :
_name(name), _type("std"), _root(root), _index(index), 
_autoIndex(false), _allowGet(allowGet), _allowPost(allowPost), _allowDel(allowDel), 
_clientMaxBodySize(clientMaxBodySize), _errorPages(errorPages)
{}

Location::Location(const std::string& name, const std::string& root, const std::string& index, 
            const bool allowGet, const bool allowPost, const bool allowDel, 
            const unsigned int clientMaxBodySize, const intStrMap errorPages, const std::string& conf)  :
_name(name), _type("std"), _root(root), _index(index), 
_autoIndex(false), _allowGet(allowGet), _allowPost(allowPost), _allowDel(allowDel), 
_clientMaxBodySize(clientMaxBodySize), _errorPages(errorPages)
{
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
        // std::cout << "key: return value:" << _path << std::endl;
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
                // std::cout << "key: " << key << " value:" << valueStr << std::endl;
                tmpVars.pop();
                continue;
            }

            if (key == "cgi_path" )
            {
                _path = valueStr;
                // std::cout << "key: " << key << " value:" << valueStr << std::endl;
                tmpVars.pop();
                continue;
            }

            if (key == "cgi_pgname" )
            {
                _pgname = valueStr;
                // std::cout << "key: " << key << " value:" << valueStr << std::endl;
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
                // std::cout << "key: " << code << " value:" << valueStr << std::endl;
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
            // std::cout << "key: " << key << " value:" << valueStr << std::endl;
            tmpVars.pop();
            continue;
        }

        if (key == "index" )
        {
            _index = valueStr;
            // std::cout << "key: " << key << " value:" << valueStr << std::endl;
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
                    _allowGet = true;
                else if (tmp == "POST")
                    _allowPost = true;
                else if (tmp == "DELETE")
                    _allowDel = true;
            }
            tmpVars.pop();
            continue;
        }

        if (key == "autoindex")
        {
            if  (valueStr == "on")
            {
                _autoIndex = true;
            }
            else if (valueStr != "off")
            {
                std::cout << "Error: invalid input: " << tmpVars.front() << std::endl;
                return;
            }
            tmpVars.pop();
            continue;
        }

        if (key == "client_max_body_size")
        {
            char*               endPtr      = NULL;
            unsigned long       tmpValue    = strtoul(valueStr.c_str(), &endPtr, 0);
            if  (endPtr == valueStr || endPtr != &(valueStr[valueStr.size()]))
            {
                std::cout << "Error: invalid input: " << tmpVars.front() << std::endl;
                return;
            }
            _clientMaxBodySize = static_cast<unsigned int>(tmpValue);
            tmpVars.pop();
            continue;
        }

        std::cout << "      Warning: unknown key: " << key << " value:" << valueStr << std::endl;
        tmpVars.pop();
    }  
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
    std::cout << "          - type          : " << me.getType() << std::endl;
    std::cout << "          - root          : " << me.getRoot() << std::endl;
    std::cout << "          - index         : " << me.getIndex() << std::endl;
    std::cout << "          - path          : " << me.getPath() << std::endl;
    std::cout << "          - pgname        : " << me.getPgName() << std::endl;
    std::cout << "          - autoindex     : " << me.isAutoIndex() << std::endl;
    std::cout << "          - get           : " << me.isGetAllowed() << std::endl;
    std::cout << "          - post          : " << me.isPostAllowed() << std::endl;
    std::cout << "          - del           : " << me.isDelAllowed() << std::endl;
    std::cout << "          - clientMax...  : " << me.getClientMaxBodySize() << std::endl;
    return(o);
}
