/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: manbengh <manbengh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 18:24:04 by manbengh          #+#    #+#             */
/*   Updated: 2025/12/08 19:34:10 by manbengh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password), _server_fd(-1)
{}

Server::~Server(){}


void Server::processPoll()
{
    pollfd pollFd;
    pollFd.fd = _server_fd;
    pollFd.events = POLLIN;
    pollFd.revents = 0;
    _pollfds.push_back(pollFd);
    
    while (true)
    {
        int nbrSockEvent = poll(_pollfds.data(), _pollfds.size(), -1);
        if (nbrSockEvent < 0)
            throw std::runtime_error("poll() failed");
            
        if (_pollfds[0].revents & POLLIN)
        {
            std::cout << "⭐ Un client frappe à la porte (POLLIN détecté) !" << std::endl;
        }
        
    }
    
}


void Server::startServ()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
        throw std::runtime_error("socket() failed");
        
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) > 0)
        throw std::runtime_error("setsockopt() failed");
    
    sockaddr_in addrServer;
    std::memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = INADDR_ANY;
    addrServer.sin_port = htons(_port);

    //bind
    if (bind(_server_fd, (sockaddr*)&addrServer, sizeof(addrServer)) < 0)
        throw std::runtime_error("bind() failed");
    
    //listen
    if (listen(_server_fd, SOMAXCONN) < 0)
        throw std::runtime_error("listen() failed");
        
    std::cout << "Server listening on port " << _port << "...\n";
    
    
    processPoll();
}

// while (true)
//     {
//         sockaddr_in addrClient;
//         socklen_t clientLen = sizeof(addrClient);

//         int clientFD = accept(_server_fd, (sockaddr*)&addrClient, &clientLen);
//         if(clientFD < 0)
//             continue;
//         std::cout << "New client connected : fd=" << clientFD << std::endl;

//         std::string hello = ": server 001 welcome to ft_irdc ma star\r\n";
        
//         send(clientFD, hello.c_str(), hello.size(), 0);

//         close(clientFD);
//     }