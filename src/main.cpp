/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jde-la-f <jde-la-f@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 10:42:20 by adcarnec          #+#    #+#             */
/*   Updated: 2023/07/31 09:35:55 by jde-la-f         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "WebServ.hpp"
#include "Utils.hpp"

int main(int ac, char** av)
{
    signal(SIGINT, SIG_IGN);
    
    if (ac == 1)
        Config::init("./conf/default.conf"); // Loading default configuration file
    else if (ac == 2)
        Config::init(av[1]); // Loading specified  configuration file
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
