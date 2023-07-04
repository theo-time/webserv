/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:03 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:05 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "VirtualServer.hpp"

const unsigned int                              Config::_clientMaxBodySize_min = 1024 * 1024;
const unsigned int                              Config::_clientMaxBodySize_max = 5 * 1024 * 1024;
unsigned int                                    Config::_clientMaxBodySize = 1024 * 1024;
bool                                            Config::_valid = false;

typedef std::queue<std::string>                 strQueue;
typedef std::map<int, std::string>              intStrMap;
typedef std::map<std::string, VirtualServer*>   srvMap;
typedef std::vector<VirtualServer*>             srvVect;

srvVect                                         Config::_virtualServers;
srvMap                                          Config::_serverNames;

strQueue                                        Config::_tmpVarConf;
strQueue                                        Config::_tmpSrvConf;
std::string                                     Config::_tmpConfData;


void Config::init(const std::string& filename)
{
    if (!checkConfFile(filename))
        return;
        
    if (_tmpSrvConf.empty())
    {
        std::cout << "Error: missing virtual server configuration" << std::endl;
        return;
    }

    while (!_tmpVarConf.empty())
    {
        addVarConf(_tmpVarConf.front());
        _tmpVarConf.pop();
    }

    std::cout << "  Found " << _tmpSrvConf.size() << " virtual server configuration(s)" << std::endl;
    int i = 0; // used to display conf number
    while (!_tmpSrvConf.empty())
    {
        addSrvConf(_tmpSrvConf.front(), ++i);
        _tmpSrvConf.pop();
    }
        
    srvVect::iterator itv = _virtualServers.begin();
    while (itv != _virtualServers.end())
    {
        std::cout << "      " << **itv << std::endl;
        itv++;
    }

    std::cout << "  Found  " << _serverNames.size() << " server name mapping(s)" << std::endl;
    srvMap::iterator it = _serverNames.begin();
    while (it != _serverNames.end())
    {
        std::cout << "      " << it-> first << " => " << it->second->getRoot() << std::endl;
        it++;
    }
    std::cout << std::endl;
    
    _valid=true;
}

std::string getFilename(std::string path)
{
    std::string filename = path.substr(path.find_last_of("/") + 1);
    return filename;
}

bool checkBracesCnt(std::string fileContent)
{

    int         nOpen   = 0;
    int         nClose  = 0;

    std::size_t found   = fileContent.find("{");
    while (found != std::string::npos)
    {
        nOpen++;
        found = fileContent.find("{", found + 1);
    }
    
    found   = fileContent.find("}");
    while (found != std::string::npos)
    {
        nClose++;
        found = fileContent.find("}", found + 1);
    }
    if (nOpen != nClose)
        return(false);

    return(true);
}

bool Config::checkConfFile(const std::string& filename)
{
    if (filename.size() < 6  || getFilename(filename).size() < 6 || filename.find(".") == std::string::npos || filename.find(".conf") != filename.size() - 5)
    {
        std::cout << "Error: invalid filename: " << filename << std::endl;
        return(false);
    }

    std::ifstream                           ifs(filename.c_str());
    if (ifs)
    {
        std::string                         line;
        std::size_t                         found;

        _tmpConfData = "";
        std::cout << "Parsing config file: " << filename << std::endl;
        while(std::getline(ifs, line))
        {
            if (!line.empty())
            {
                // clear comments
                found = line.find('#');
                if (found != std::string::npos)
                    line.erase(line.begin() + found, line.end());

                _tmpConfData = _tmpConfData + line;
                
            }
        }
        ifs.close();

        if (_tmpConfData.empty())
        {
            std::cout << "Error: empty configuration file: " << filename << std::endl;
            return(false);
        }

        if (!checkBracesCnt(_tmpConfData))
        {
            std::cout << "Error: inconsistent braces: " << filename << std::endl;
            return(false);
        }

        //remove white spaces and end of line
        std::string::iterator it = _tmpConfData.begin();
        while (it != _tmpConfData.end())
        {
            if (*it == '\n' || *it == ' ' || (*it >= 9 && *it <= 13))
                _tmpConfData.erase(it);
            else
                it++;
        }
        if (_tmpConfData.empty())
        {
            std::cout << "Error: empty configuration file: " << std::endl;
            return(false);
        }
        
        //check empty braces
        found = _tmpConfData.find("{}");
        if (found != std::string::npos)
        {
            std::cout << "Error: found empty braces: " << std::endl;
            return(false);
        }
        
        //check forbidden char
        found = _tmpConfData.find_first_of("*+<>|"); //TODO tbc
        if (found != std::string::npos)
        {
            std::cout << "Error: found forbidden characters (" << found << "): " << std::endl;
            std::cout << _tmpConfData << std::endl;
            return(false);
        }

        return(parseConfData());
    }
    else
    {
        std::cout << "Error: could not open file: " << filename << std::endl;
        return(false);
    }
}

bool Config::parseConfData()
{
    std::string                         srvConf = "";
    bool                                srv = false; 
    std::size_t                         found;

    while (!_tmpConfData.empty())
    {
        if (!srv)
        {
            found = _tmpConfData.find("server{");
            if (found == 0)
            {
                srv = true;
                _tmpConfData.erase(0, 7);
                continue;
            }

            found = _tmpConfData.find(";");
            if (found == std::string::npos)
            {
                std::cout << "Error: missing ';' token: " << _tmpConfData << std::endl;
                return(false);
            }
            _tmpVarConf.push(_tmpConfData.substr(0, found + 1));
            _tmpConfData.erase(0, found + 1);
        }
        else
        {
            found = _tmpConfData.find("}");
            if (found == std::string::npos)
            {
                std::cout << "Error: missing closing brace for virtual server configuration: " << _tmpConfData << std::endl;
                return(false);
            }

            std::size_t location = _tmpConfData.find("location=");
            if (location == std::string::npos || location > found)
            {
                srvConf = srvConf + _tmpConfData.substr(0, found);
                _tmpSrvConf.push(srvConf);
                srvConf.clear();
                srv = false;
            }
            else
            {
                srvConf = srvConf + _tmpConfData.substr(0, found + 1);
            }
            _tmpConfData.erase(0, found + 1);
        }
    }
    return(true);
}

void Config::addVarConf(std::string& line)
{
    std::size_t sep = line.find('=');
    std::size_t end = line.find(';');
    if (sep == std::string::npos || end == std::string::npos || sep == 0 || sep + 1 == end)
    {
        std::cout << "Error: invalid input: " << line << std::endl;
        return;
    }
    std::string key = line.substr(0, sep);
    std::string valueStr = line.substr(sep + 1, line.length() - sep - 2);

    if(key == "client_max_body_size")
    {
        char*               endPtr      = NULL;
        unsigned long       tmpValue = strtoul(valueStr.c_str(), &endPtr, 0);
        if  (endPtr == valueStr || endPtr != &(valueStr[valueStr.size()]) 
        || tmpValue > _clientMaxBodySize_max || tmpValue < _clientMaxBodySize_min)
        {
            std::cout << "Error: invalid input: " << line << std::endl;
            return;
        }
        _clientMaxBodySize = static_cast<unsigned int>(tmpValue);
        std::cout << "  adding key = " << key;
        std::cout << ", value = " << _clientMaxBodySize << std::endl;
        return;
    }

    std::cout << "  Warning: unknown key: " << line << std::endl;
}

void Config::addSrvConf(std::string& line, int i)
{
    std::cout << "  Parsing config #" << i << std::endl;
    
    strQueue        tmpVars, tmpLocations;
    std::string     tmpLine = line;
    std::size_t     sep;

    while (!line.empty())
    {
        sep = line.find("location=");
        if (sep == 0)
        {
            line.erase(0, 9);
            sep = line.find("}");
            if (sep == std::string::npos)
            {
                std::cout << "  Error: missing closing brace for location configuration" << tmpLine << std::endl;
                return;
            }
            tmpLocations.push(line.substr(0, sep));
            line.erase(0, sep + 1);
            continue;
        }

        sep = line.find(';');
        tmpVars.push(line.substr(0, sep));
        line.erase(0, sep + 1);
    }

    std::string     portStr = "";
    std::string     serverName = "";
    std::string     maxBodySizeStr = "";
    unsigned int    port = -1;
    unsigned int    maxBodySize = -1;
    intStrMap       tmpErrorPages;
    std::string     index = "";
    std::string     root = "";
    std::string     host = "";
    bool            isGetAllowed = false;
    bool            isPostAllowed = false;
    bool            isDelAllowed = false;
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
}
    
void Config::clear(void)
{
    if (!_virtualServers.empty ())
    {
        srvVect::iterator it    = _virtualServers.begin();
        srvVect::iterator end   = _virtualServers.end();

        while (it != end)
        {
            delete *it;
            it++;
        }
    }
}

unsigned int& Config::getClientMaxBodySize(void)
{
    return(_clientMaxBodySize);
}

bool& Config::isValid(void)
{
    return(_valid);
}

srvVect& Config::getVirtualServers(void)
{
    return(_virtualServers);
}
