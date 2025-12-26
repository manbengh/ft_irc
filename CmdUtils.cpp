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


void Server::handleTopic(int fd, std::string chanName, std::string topic)
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
