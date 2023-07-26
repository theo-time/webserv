/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jde-la-f <jde-la-f@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 10:42:20 by adcarnec          #+#    #+#             */
/*   Updated: 2023/07/26 17:16:48 by jde-la-f         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <csignal>
#include <cstring>
#include <string>
#include <cerrno>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Config.hpp"
#include "WebServ.hpp"
#include "Utils.hpp"

void    ignoreSig(int sig);
static void    doExit(int epollFd);
static int     initEpoll(void);
static int     runListeners(int epollFd);
static int     runListener(int port);
static int     addEvent(int epollFd, int fd, int state);
static int     doEpoll(int epollFd);

int main(int ac, char** av)
{
    // intercept Ctrl+C
    // signal(SIGINT, ignoreSig);

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
        int epollFd = initEpoll();
        if (epollFd != -1)
            runListeners(epollFd);
        doExit(epollFd);
    }
    else
    {
        if (!WebServ::runListeners())
                WebServ::stop();
    } 

    return 0;
}

void ignoreSig(int sig)
{
    (void)sig;
    char c;
    std::cin >> c;
    signal(SIGINT,SIG_DFL);
}

static void doExit(int epollFd)
{
    close(epollFd);
}

static int initEpoll(void)
{ 
    // create epoll instance
    int epollFd = epoll_create(1); //argument is ignored, but must be greater than zero

    if (epollFd == -1)
        strerror(errno);

    return(epollFd);
}

static int runListeners(int epollFd)
{
    //add stdin
    addEvent(epollFd, STDIN_FILENO, EPOLLIN);

    // TODO loop Config::_virtualServers

    int     port = 8080;
    int listenFd = runListener(port);
    if (listenFd == -1)
    {
        strerror(errno);
        return(-1);
    }
    addEvent(epollFd, listenFd, EPOLLIN);

    // loop epoll_wait
    doEpoll(epollFd);

    return(0);
}

static int runListener(int port)
{
    int     listenQ = 32; //maximum length to which the queue of pending connections for socket may grow

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd == -1)
    {
        strerror(errno);
        return(-1);
    }

    struct sockaddr_in  srvAddr;
    ft_bzero(&srvAddr, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_addr.s_addr = INADDR_ANY;
    srvAddr.sin_port = htons(port);
    if (bind(listenFd, (struct sockaddr*)&srvAddr, sizeof(srvAddr)) == -1) // bind the socket to port number
    {
        strerror(errno);
        close(listenFd);
        return(-1);
    }

    // Allow socket descriptor to be reuseable 
    int on = 1;
    if (setsockopt(listenFd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) == -1)
    {
        strerror(errno);
        close(listenFd);
        return(-1);
    }

    // Set socket to be nonblocking
    if (fcntl(listenFd, F_SETFL, O_NONBLOCK) == -1)
    {
        strerror(errno);
        close(listenFd);
        return(-1);
    }

    if (listen(listenFd, listenQ) == -1) // wait for connections
    {
        strerror(errno);
        close(listenFd);
        return(-1);
    }

    return(listenFd);
}
    
static int addEvent(int epollFd, int fd, int state)
{
    struct epoll_event  ev;

    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        strerror(errno);
        return(-1);
    }
    return(0);
}

static int doEpoll(int epollFd)
{
    int                 nReady;
    int                 epollEvents = 100;
    struct epoll_event  events[epollEvents];

    while (true)
    {
        nReady = epoll_wait(epollFd, events, epollEvents, -1);
        if (nReady == -1)
        {
            strerror(errno);
            break;
        }
        for (int i = 0; i < nReady; i++)
        {
            int fd = events[i].data.fd;
            if (fd == STDIN_FILENO && (events[i].events & EPOLLIN))
            {
                std::string buffer;
                std::getline(std::cin, buffer);

                if (buffer == "EXIT")
                    return(-1);
            }
/*             else if (fd == listenFd && (events[i].events & EPOLLIN))
                acceptCnx(epollFd, fd);
            else if (events[i].events & EPOLLIN)
                readRequest(epollFd, fd);
            else if (events[i].events & EPOLLOUT)
                sendResponse(epollFd, fd, response.c_str()); */
        }
    }
    //doExit(epollFd);
    return(0);
}
