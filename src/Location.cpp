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

Location::Location(const std::string& conf) :
_name("tmp"), _autoIndex(false), _allowGet(false), _allowPost(false), _allowDel(false), _clientMaxBodySize(Config::getClientMaxBodySize())
{
    (void)conf;
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

std::string Location::getExtension(void) const
{
    return(_extension);
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
