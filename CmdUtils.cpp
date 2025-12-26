#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"


bool Channel::isOperator(int fd) const
{
    std::map<int,bool>::const_iterator it = _clientCh.find(fd);
    if (it != _clientCh.end())
        return it->second; // true = operateur
    return false;
}

void Channel::setOperator(int fd, bool isOp)
{
    std::map<int, bool>::iterator it = _clientCh.find(fd);
    if (it != _clientCh.end())
        it->second = isOp;
}





void Server::ftTopic(int fd, std::string chanName, std::string topic)
{
    Client &client = _clients[fd];
    if (!client.isRegistered())
    {
        std::string err = "451 :You have not registered\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return ;
    }

    if (chanName.empty() || chanName.find('#'))
        return;

    if (_channels.find(chanName) == _channels.end())
    {
        std::string err = ":server 403 " + chanName + " :No such channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    Channel &chan = _channels[chanName];
    if (!chan.hasClient(fd))
    {
        std::string err = ":server 442 " + chanName + " :You're not on that channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    if (topic.empty())
    {
        if (chan.getTopic().empty())
        {
            std::string allMsg =   ":server 331 " + client.getNick() +
                                    " " + chanName + " :No topic is set\r\n";
            send(fd, allMsg.c_str(), allMsg.size(), 0);
        }
        else 
        {
            std::string allMsg =   ":server 332 " + client.getNick() +
                                    " " + chanName + " :" + chan.getTopic() + "\r\n";
            send(fd, allMsg.c_str(), allMsg.size(), 0);
        }
        return;
    }

    if (!chan.isOperator(fd))
    {
        std::string err = ":server 482 " + chanName + " :You're not channel operator\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
    }

    chan.setTopic(topic);
    std::string topicMsg = ":" + client.getNick() + " TOPIC " +
                           chanName + " :" + topic + "\r\n";
    
    for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
    {
        // if (it->first != fd) // pas envoyer au client qui envoie (pas sur)
        send(it->first, topicMsg.c_str(), topicMsg.size(), 0);
    }
}


void Server::ftKick(int fd, std::string &name, std::string &chanName)
{
    Client &client = _clients[fd];
    if (!client.isRegistered())
    {
        std::string err = "451 :You have not registered\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return ;
    }

    if (chanName.empty() || chanName[0] != '#')
        return;

    if (_channels.find(chanName) == _channels.end())
    {
        std::string err = ":server 403 " + chanName + " :No such channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    Channel &chan = _channels[chanName];
    if (!chan.hasClient(fd))
    {
        std::string err = ":server 442 " + chanName + " :You're not on that channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    if (!chan.isOperator(fd))
    {
        std::string err = ":server 482 " + chanName + " :You're not channel operator\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
    }

    int targetFd = -1;
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNick() == name)
        {
            targetFd = it->first;
            break;
        }
    }
    if (targetFd == -1)
    {
        std::string err = ":server 401 " + name + " :k\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    if (!chan.hasClient(targetFd))
    {
        std::string err = ":server 441 " + name + " " + chanName + " :They aren't on that channel\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
    }
    
    std::string kickMsg = ":" + client.getNick() + " KICK " +
                           chanName + " :" + name + " :kicked\r\n";
    
    for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
    {
        // if (it->first != fd) // pas envoyer au client qui envoie (pas sur)
        send(it->first, kickMsg.c_str(), kickMsg.size(), 0);
    }
    chan.removeClient(targetFd);
    std::cout << "👢 " << name << " kicked from " << chanName
              << " by " << client.getNick() << std::endl;
}
