/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahbey <ahbey@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/05 18:23:44 by manbengh          #+#    #+#             */
/*   Updated: 2025/12/27 16:18:54 by ahbey            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int parseInput(int port, std::string password)
{
    (void)password;
    if (port <= 0 || port > 65535)
        return 1;
    return 0;
}


int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Error : ./ircserv <port> <password>\n";
        return 1;
    }
    

    try
    {
        int port = std::atoi(av[1]);
        std::string password = av[2];
        if (parseInput(port, password))
        {
            std::cerr << "Error : invalid arguments\n" << std::endl;
            return 1;
        }
        
        Server server(port, password);
        server.startServ();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}