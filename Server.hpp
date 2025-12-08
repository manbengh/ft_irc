#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdlib>
#include <poll.h>
#include <vector>


class Server
{
    private:
        int _port;
        std::string _password;
        int _server_fd;
        std::vector<pollfd> _pollfds;

    public:
        Server(int port, std::string password);
        ~Server();

        void startServ();
        void processPoll();

};


#endif