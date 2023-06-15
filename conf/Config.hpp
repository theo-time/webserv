#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <cstdlib>
# include <fstream>
# include <string>
# include <cctype>
# include <vector>
# include <queue>
# include <map>

class Config
{
    public:

        typedef std::map<std::string, std::string>  map;
        typedef std::queue<std::string>             queue;

        class Server
        {
            public:
                Server(const unsigned int& port);
                ~Server(void);

            private:
                unsigned int        _port;
                std::string         _names;
                std::string         _index;
                std::string         _root;
                map                 _errorPages;
        };

        typedef std::vector<Config::Server>         vectSrv;

        static bool                 init(const std::string& filename);

    private:
        static unsigned int         _clientMaxBodySize;
        static vectSrv              _srvConfs;
        static map                  _srvList;
        static queue                _tmpVar;
        static queue                _tmpSrv;

        static bool                 check(const std::string& filename);
        static void                 addMainContext(std::string& line);
        static void                 addServer(std::string& line);
};

#endif
