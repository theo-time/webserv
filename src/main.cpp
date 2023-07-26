#include <iostream>
#include "Config.hpp"
#include "WebServ.hpp"

int main(int ac, char** av)
{
    if (ac == 1)
        Config::init("./conf/default.conf");
    else if (ac == 2)
        Config::init(av[1]);
    else {
        std::cout << "Error: too many arguments" << std::endl;
        return(-1);
    }

    if (!Config::isValid())
        return(-1);

    if (!WebServ::runListeners())
        WebServ::stop();

    return 0;
}
