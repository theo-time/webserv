/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <string>
# include <queue>
# include <map>

# include "Config.hpp"


class Location
{
    public:

        typedef std::queue<std::string>         strQueue;
        typedef std::map<int, std::string>      intStrMap;

        Location(const std::string& name, const std::string& root, const std::string& index, 
            const bool allowGet, const bool allowPost, const bool allowDel, 
            const unsigned int clientMaxBodySize, intStrMap& errorPages);
        Location(const std::string& name, const std::string& root, const std::string& index, 
            const bool allowGet, const bool allowPost, const bool allowDel, 
            const unsigned int clientMaxBodySize, intStrMap& errorPages, const std::string& conf);
        ~Location(void);
        
        Location& operator=(const Location& rhs);

        std::string         getName(void) const;
        std::string         getType(void) const;
        std::string         getRoot(void) const;
        std::string         getIndex(void) const;
        std::string         getPath(void) const;
        std::string         getExtension(void) const;
        std::string         getPgName(void) const;
        unsigned int        getClientMaxBodySize(void) const;
        intStrMap&          getErrorPages(void);
        bool                isGetAllowed(void) const;
        bool                isPostAllowed(void) const;
        bool                isDelAllowed(void) const;
        bool                isAllowed(std::string& method) const;
        bool                isAutoIndex(void) const;

    private:
/*     
        Location(void);
        Location(const Location& src);
 */
        std::string         _name;
        std::string         _type;
        std::string         _root;
        std::string         _index;
        std::string         _path;
        std::string         _extension;
        std::string         _pgname;
        bool                _autoIndex;
        bool                _allowGet;
        bool                _allowPost;
        bool                _allowDel;
        unsigned int        _clientMaxBodySize;
        intStrMap&          _errorPages;
};

std::ostream& operator<<(std::ostream& o, Location& me);

#endif
