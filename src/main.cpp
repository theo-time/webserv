#include <iostream>
#include "WebServ.hpp"

int main(int ac, char** av)
{
    if (ac == 1) {
        if (!WebServ::init("./conf/default.conf"))
            return(-1);
    }
    else if (ac == 2) {
        if (!WebServ::init(av[1]))
            return(-1);
    }
    else {
        std::cout << "Error: too many arguments" << std::endl;
        return(-1);
    }

    if (!WebServ::runListeners())
        return(-1);
        
    WebServ::stop();
    return 0;
}
