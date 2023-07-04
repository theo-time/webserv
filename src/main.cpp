#include <iostream>
#include "Config.hpp"
#include "WebServ.hpp"



#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
void printDir() {
    DIR             *dir;
    struct dirent   *entry;

    if ((dir = opendir("./data/default")) == NULL)
        perror("opendir() error");
    else {
        printf("contents of ./data/default:\n");
        while ((entry = readdir(dir)) != NULL)
            printf("  %s\n", entry->d_name);
        printf("\n");
        closedir(dir);
    }
}


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

    printDir(); // TODO delete

    if (!Config::isValid())
        return(-1);

    if (!WebServ::runListeners())
        WebServ::stop();

    return 0;
}
