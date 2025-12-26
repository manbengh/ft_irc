#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"
#include <map>

class Channel
{
    private :
                std::string _name;
                std::string _topic;
                std::string _passwordCh;

                std::map<int, bool> _clientCh;
                std::map<int, bool> _invited;


    public :
                Channel(){};
                Channel(std::string &name): _name(name){}
                ~Channel();

                void addClient(int fd, bool isOper);
                void removeClient(int fd);
                bool hasClient(int fd)const;
                const std::map<int, bool> &getClients()const;
                bool isOperator(int fd) const;

                const std::string &getTopic() const;
                void setTopic(const std::string &topic);

};



#endif
