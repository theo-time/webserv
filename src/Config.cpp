/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:03 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:05 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

const unsigned int                          Config::_clientMaxBodySize_min = 1024 * 1024;
const unsigned int                          Config::_clientMaxBodySize_max = 5 * 1024 * 1024;
unsigned int                                Config::_clientMaxBodySize = 1024 * 1024;
bool                                        Config::_valid = false;

typedef std::queue<std::string>             queue;
typedef std::vector<VirtualServer*>         srvVect;

srvVect                                     Config::_virtualServers;
queue                                       Config::_tmpVarConf;
queue                                       Config::_tmpSrvConf;
std::string                                 Config::_tmpConfData;

bool Config::isValid(void)
{
    return(_valid);
}

srvVect& Config::getVirtualServers(void)
{
    return(_virtualServers);
}

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
    while (!_tmpSrvConf.empty())
    {
        addSrvConf(_tmpSrvConf.front());
        _tmpSrvConf.pop();
    }

    _valid=true;
}

bool Config::checkConfFile(const std::string& filename)
{
    if (filename.size() < 6  || filename.find(".") == std::string::npos || filename.find(".conf") != filename.size() - 5)
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
        if (_tmpConfData.empty())
        {
            std::cout << "Error: empty configuration file: " << filename << std::endl;
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
            std::cout << "Error: empty configuration file: " << filename << std::endl;
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

void Config::addSrvConf(std::string& line)
{
    std::cout << "  parsing server config:" << std::endl;
    std::cout << line << std::endl;
    
    queue           tmpVars, tmpLocations;
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

    std::string     portStr;
    unsigned int    port;
    std::string     index, root, host = "";
    bool            isGetAllowed, isPostAllowed, isDelAllowed = false;
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
            std::cout << "      key = " << key;
            std::cout << ", value = " << port << std::endl;
            tmpVars.pop();
            continue;
        }

        if(key == "host")
        {
            host = valueStr;
            std::cout << "      key = " << key;
            std::cout << ", value = " << valueStr << std::endl;
            tmpVars.pop();
            continue;
        }

        if(key == "index")
        {
            index = valueStr;
            std::cout << "      key = " << key;
            std::cout << ", value = " << index << std::endl;
            tmpVars.pop();
            continue;
        }

        if(key == "root")
        {
            root = valueStr;
            std::cout << "      key = " << key;
            std::cout << ", value = " << root << std::endl;
            tmpVars.pop();
            continue;
        }

        if(key == "methods")
        {
            std::cout << "      key = " << key;
            std::cout << ", value = " << valueStr << std::endl;
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
        

        std::cout << "      Warning: unknown key: " << tmpVars.front() << std::endl;
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
    _virtualServers.push_back(tmp);
}


/*     if (srv)
    {
        if (!location)
        {
            found = line.find("location=");
            if (found != std::string::npos)
            {
                location = true;
            }
        }

        found = line.find('}');
        if (found != std::string::npos)
        {
            if (location)
                location = false;
            else
            {
                line.erase(line.begin() + found, line.end()); // TODO a checker
                srv = false;
            }
        }
        srvConf = srvConf + line;
        if (!srv)
        {
            std::cout << "srvConf : " << srvConf << std::endl;
            _tmpSrvConf.push(srvConf);
            srvConf.clear();
        }
    }
    else
    {
        found = line.find("server{");
        if (found == 0)
            srv = true;
        else
            _tmpVarConf.push(line);
    } */
/*     
    if (!location && !srv)
        return(true);
    else
    {
        std::cout << "Error: missing closing brace for virtual server configuration: " << srvConf << std::endl;
        return(false);
    } */
    
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

unsigned int Config::getClientMaxBodySize(void)
{
    return(_clientMaxBodySize);
}
