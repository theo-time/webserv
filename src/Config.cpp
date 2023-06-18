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

unsigned int                                Config::_clientMaxBodySize = 1024 * 1024;
bool                                        Config::_valid = false;

typedef std::queue<std::string>             queue;
typedef std::vector<VirtualServer*>         srvVect;

srvVect                                     Config::_virtualServers;
queue                                       Config::_tmpVarConf;
queue                                       Config::_tmpSrvConf;

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

    std::cout << "Found " << _tmpSrvConf.size() << " virtual server configuration(s)" << std::endl;
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
        std::string                         srvConf;
        bool                                srv = false; 

        std::cout << "Parsing config file: " << filename << std::endl;
        while(std::getline(ifs, line))
        {
            if (!line.empty())
            {
                // clear comments
                std::size_t found = line.find('#');
                if (found != std::string::npos)
                    line.erase(line.begin() + found, line.end());

                // trim spaces
                while (isspace(line[0]))
                    line.erase(0, 1);
                if (line.empty())
                    continue;
                while (isspace(line[line.length() - 1]))
                    line.erase(line.length() - 1, 1);
                if (line.empty())
                    continue;

                if (srv)
                {
                    found = line.find('}');
                    if (found != std::string::npos)
                    {
                        line.erase(line.begin() + found, line.end());
                        srv = false;
                    }
                    srvConf = srvConf + line;
                    if (!srv)
                    {
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
                }
            }
        }
        if (!srv)
            return(true);
        else
        {
            std::cout << "Error: missing closing brace for virtual server configuration: " << srvConf << std::endl;
            return(false);
        }
    }
    else
    {
        std::cout << "Error: could not open file: " << filename << std::endl;
        return(false);
    }
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
        || tmpValue > 5 * 1024 * 1024 || tmpValue < 1024 * 1024)
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
    std::cout << "  parsing Server..." << std::endl;
    
    queue           tmpVars;
    std::string     tmpLine = line;
    std::size_t     sep = tmpLine.find(';');
    while (sep != std::string::npos)
    {
        tmpVars.push(tmpLine.substr(0, sep));
        tmpLine.erase(0, sep + 1);
        sep = tmpLine.find(';');
    }

    std::string     portStr;
    unsigned int    port;
    std::string     index, root = "";
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

        std::cout << "      Warning: unknown key: " << tmpVars.front() << std::endl;
        tmpVars.pop();
    }

    if (portStr.empty())
    {
        std::cout << "      Error: missing port number" << std::endl;
        return;
    }

    VirtualServer* tmp = new VirtualServer(port, root);        
    _virtualServers.push_back(tmp);
}
