/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: manbengh <manbengh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 18:24:04 by manbengh          #+#    #+#             */
/*   Updated: 2025/12/05 18:56:17 by manbengh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password), _server_fd(-1)
{}

Server::~Server(){}


void Server::startServ()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
        throw std::runtime_error("socket() failed");
        
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) > 0)
        throw std::runtime_error("setsockopt() failed");
    
}
