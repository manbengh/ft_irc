#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


class Server
{
    private:
        int _port;
        std::string _password;
        int _server_fd;
        

    public:
        Server(int port, std::string password);
        ~Server();

        void startServ();

};


#endif