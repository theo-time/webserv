/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 10:42:20 by adcarnec          #+#    #+#             */
/*   Updated: 2023/07/26 10:42:28 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Config.hpp"
#include "WebServ.hpp"

int main(int ac, char** av)
{
    // TODO del qd epoll ok
    std::string arg;
    
    if (ac == 1)
        Config::init("./conf/default.conf"); // Loading default configuration file
    else if (ac == 2)
    {
        // TODO del qd epoll ok
        arg = av[1];
        if (arg == "EPOLL")
        {
            Config::init("./conf/default.conf");
        }
        else
            Config::init(av[1]); // Loading specified  configuration file
    }
    else {
        std::cout << "Error: too many arguments" << std::endl;
        return(-1);
    }

    if (!Config::isValid())
        return(-1);

    // TODO del qd epoll ok
    if (arg == "EPOLL")
    {
        std::cout << "EPOLL" << std::endl;
    }
    else
    {
        if (!WebServ::runListeners())
                WebServ::stop();
    } 

    return 0;
}
