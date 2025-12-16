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
            sockaddr_in addrClient;
            socklen_t clientLen = sizeof(addrClient);


            int clientFD = accept(_server_fd, (sockaddr*)&addrClient, &clientLen);
            if (clientFD < 0)
            {
                std::cerr << "accept() failed\n";
                continue ;
            }
            std::cout << "⭐ Nouveau client connecté : fd=" << clientFD << std::endl;
 
            pollfd clientPoll;
            clientPoll.fd = clientFD;
            clientPoll.events = POLLIN;
            clientPoll.revents = 0;
            _pollfds.push_back(clientPoll);
            
            _clients[clientFD] = Client(clientFD);//
            std::string err = "Welcom to our FT_IRC, please put your : PASSWORD, NICK, USER\r\n";
            send(clientFD, err.c_str(), err.size(), 0);

        }
        _pollfds[0].revents = 0;
        for (size_t i = 1; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                char buffer[512];
                int fd = _pollfds[i].fd;
                int bytes = recv(_pollfds[i].fd, buffer, sizeof(buffer) - 1, 0);// lis les donne envoyer par les clicli
            
                if (bytes <= 0)
                {            
                    std::cout << "❌ Client fd=" << _pollfds[i].fd << " déconnecté\n";
                    close(fd);
                    _clients.erase(fd);
                    close(_pollfds[i].fd);
                    _pollfds.erase(_pollfds.begin() + i);
                    i--;
                    continue;
                }
                buffer[bytes] = '\0';

                std::string &clientBuff = _clients[fd].getBuffer();
                clientBuff.append(buffer);
                cmdIdentify(clientBuff, fd);
                _pollfds[i].revents = 0;
            }

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
    addrServer.sin_family = AF_INET;//IPv4 
    addrServer.sin_addr.s_addr = INADDR_ANY;// Le serveur ecoute ttes addr dispo
    addrServer.sin_port = htons(_port); //mettre bon format pour le systeme

    //bind
    if (bind(_server_fd, (sockaddr*)&addrServer, sizeof(addrServer)) < 0) // attacher la socket srvFd a addr/port 
        throw std::runtime_error("bind() failed");
    
    //listen
    if (listen(_server_fd, SOMAXCONN) < 0)// somax = val max auto par syst
        throw std::runtime_error("listen() failed");
        
    std::cout << "Server listening on port " << _port << "...\n";
    
    processPoll();
    
}
