
#include "Server.hpp"
#include <csignal>

Server* g_server = NULL;

int parseInput(int port, std::string password)
{
    (void)password;
    if (port <= 0 || port > 65535)
        return 1;
    return 0;
}

void ctrlSignal(int sig)
{
    if(g_server)
    {
        std::cout << "rouh tgawat toi et les fd" << sig << "bye\n";
        delete g_server;
        g_server = NULL;
    }
    exit(1);
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
        std::signal(SIGINT, ctrlSignal);
        std::signal(SIGPIPE, SIG_IGN);
        
        g_server = new Server(port, password);
        g_server->startServ();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        delete g_server;
        return 1;
    }
    delete g_server;
    return 0;
}