/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientCnx.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 14:37:16 by adcarnec          #+#    #+#             */
/*   Updated: 2023/06/13 14:37:20 by adcarnec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ClientCnx.hpp"

ClientCnx::ClientCnx(const ClientCnx& src) : 
_fd(src.getFd()), _clientAddress(src.getClientAddress()), _server(src.getServer())
{
    *this = src;
}

ClientCnx::ClientCnx(const int& fd, struct sockaddr_in& clientAddress) :
_fd(fd), _clientAddress(clientAddress) 
{   
    std::string         path = "./data/default/index.html";    
    _response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Disposition: inline; filename=\"index.html\"\r\nContent-Length: " ;
    WebServ::getRessource(path, *this);
}

ClientCnx::ClientCnx(void){}
ClientCnx::~ClientCnx(void){}

ClientCnx& ClientCnx::operator=(const ClientCnx& rhs)
{
    if (this != &rhs)
    {
        _fd = rhs.getFd() ;
        _clientAddress = rhs.getClientAddress();
        _server = rhs.getServer();
        _response = rhs.getResponse();
    }
    return(*this);
}

void ClientCnx::setFileContent(std::string& fileContent)
{
    std::stringstream   ss;
    std::string         contentLength;

    ss << fileContent.length();
    ss >> contentLength;

    _response = _response + contentLength + "\r\n\r\n" + fileContent;
}

std::string ClientCnx::getResponse(void) const
{
    return(_response);
}

void ClientCnx::newRequest(std::string req)
{
    std::cout << req << std::endl;
}

void ClientCnx::setFd(int fd)
{
    _fd = fd;
}

int ClientCnx::getFd(void) const
{
    return(_fd);
}

void ClientCnx::setClientAddress(struct sockaddr_in clientAddress)
{
    _clientAddress = clientAddress;
}

struct sockaddr_in ClientCnx::getClientAddress(void) const
{
    return(_clientAddress);
}

void ClientCnx::setServer(VirtualServer& server)
{
    _server = server;
}

VirtualServer ClientCnx::getServer(void) const
{
    return(_server);
}
