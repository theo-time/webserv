#include "Config.hpp"

typedef std::map<std::string, std::string>  map;
typedef std::queue<std::string>             queue;
typedef std::vector<Config::Server>         vectSrv;

queue                                       Config::_tmpVar;
queue                                       Config::_tmpSrv;
unsigned int                                Config::_clientMaxBodySize = 1024 * 1024;
vectSrv                                     Config::_srvConfs;

Config::Server::Server(const unsigned int& port) : _port(port){}

Config::Server::~Server(void){}

bool Config::check(const std::string& filename)
{
    if (filename.size() < 6  || filename.find(".") == std::string::npos || filename.find(".conf") != filename.size() - 5)
    {
        std::cout << "Error: invalid filename: " << filename << std::endl;
        return(false);
    }

    std::ifstream       ifs(filename.c_str());
    if (ifs)
    {
        std::cout << "Parsing config file: " << filename << std::endl;
        std::string line;
        std::string srvConf;
        bool        srv = false; 

        while(std::getline(ifs, line))
        {   // TODO verifier accolades
            if (!line.empty())
            {
                while (isspace(line[0]))
                    line.erase(0, 1);
                while (isspace(line[line.length() - 1]))
                    line.erase(line.length() - 1, 1);

                std::size_t found = line.find('#');
                if (found != std::string::npos)
                    line.erase(line.begin() + found, line.end());

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
                        _tmpSrv.push(srvConf);
                        srvConf.clear();
                    }
                }
                else
                {
                    found = line.find("server{");
                    if (found == 0)
                        srv = true;
                    else
                        _tmpVar.push(line);
                }
            }
        }
        return(true);
    }
    else
    {
        std::cout << "Error: could not open file: " << filename << std::endl;
        return(false);
    }
}

void Config::addMainContext(std::string& line)
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
        std::cout << "  key = " << key;
        std::cout << ", value = " << _clientMaxBodySize << std::endl;
        return;
    }

    std::cout << "Warning: unknown key: " << line << std::endl;
}

void Config::addServer(std::string& line)
{
    std::cout << "  adding Server..." << std::endl;
    
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
    std::string     index, root;
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
        std::cout << "Error: missing port number: " << line << std::endl;
        return;
    }

    Server tmp(port);
    _srvConfs.push_back(tmp);
}

bool Config::init(const std::string& filename)
{
    if (!check(filename))
        return(false);

    while (!_tmpVar.empty())
    {
        addMainContext(_tmpVar.front());
        _tmpVar.pop();
    }

    while (!_tmpSrv.empty())
    {
        addServer(_tmpSrv.front());
        _tmpSrv.pop();
    }

    std::cout << "  _srvConfs.size() = " << _srvConfs.size() << std::endl;

    return(true);
}
