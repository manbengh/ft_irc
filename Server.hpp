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
#include "Client.hpp"
#include "Channel.hpp"
#include <map>
#include <sstream>
#include <fcntl.h>

class Client;
class Channel;  
class Server
{
    private:
        int _port;
        std::string _password;
        int _server_fd;
        std::vector<pollfd> _pollfds;
        std::map<int, Client> _clients;
        static bool _sig;

        std::map<std::string, Channel> _channels;

    public:
        Server(int port, std::string password);
        ~Server();

        void startServ();
        void processPoll();
        void passCmd(std::string pass, int fd);
        void nickCmd(std::string nick, int fd);
        void cmdIdentify(std::string &clientBuff, int fd);
        void ftJoin(int fd, std::string chanName, std::string key);
        void ftPrivMsg(int fd, std::string target, std::string msg);
        // void ftQuit(int fd, std::string reason);
        void ftPart(int fd, std::string chanName, std::string reason);
        void ftTopic(int fd, std::string chanName, std::string remains);
        void ftInvite(int fd, std::string &name, std::string &chanName);
        void ftKick(int fd, std::string &name, std::string &chanName);
        void ftMode(int fd, std::string &name, std::string &chanName, std::vector<std::string> &params);
        int findClientByNick(std::string &nick);

        static void signalHandler(int signum);
        void closeServer();
};


#endif