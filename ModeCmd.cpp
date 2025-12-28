#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"


int Server::findClientByNick(std::string &nick)
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNick() == nick)
            return it->first;
    }
    return -1;
}

std::string Channel::getModesString()
{
    std::string modes = "+";

    if (_inviteOnly)
        modes += "i";
    if (_topicRestricted)
        modes += "t";
    if (!_passwordCh.empty())
        modes += "k";
    if (hasLimit())
        modes += "l";

    return modes;
}



void Server::ftMode(int fd, std::string &chanName, std::string &modes, std::vector<std::string> &params)
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

    if (modes.empty())
    {
        std::string reply = ":server 324 " + client.getNick() + " " + chanName +
                            " " + chan.getModesString() + "\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (!chan.isOperator(fd))
    {
        std::string err = ":server 482 " + chanName + " :You're not channel operator\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
    }

    bool isItPlus = true;
    size_t paramIndex = 0;

    for (size_t i = 0; i < modes.size(); i++)
    {
        int c = modes[i];
        if (c == '+')
        {
            isItPlus = true;
            continue;
        }
        if (c == '-')
        {
            isItPlus = false;
            continue;
        }

        if (c == 'i')
            chan.setInviteOnly(isItPlus);

        else if (c == 't')
            chan.setTopicRestricted(isItPlus);

        else if (c == 'k')
        {
            if (isItPlus)
            {
                if (paramIndex >= params.size())
                {
                    std::string err = ":server 461 MODE :Not enough parameters\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    return;
                }
                chan.setPass(params[paramIndex++]);
            }
            else
                chan.removePass();
        }

        else if (c == 'o')
        {
            if (paramIndex >= params.size())
            {
                std::string err = ":server 461 MODE :Not enough parameters\r\n";
                send(fd, err.c_str(), err.size(), 0);
                return;
            }
            int targetFD = findClientByNick(params[paramIndex]);
            // paramIndex++;
            if (targetFD == -1)
            {
                std::string err = ":server 401 " + params[paramIndex] + " :No such nick\r\n";
                send(fd, err.c_str(), err.size(), 0);
                return;
            }
            if (!chan.hasClient(targetFD))
            {
                std::string err = ":server 441 " + params[paramIndex] + " " + chanName +
                                " :They aren't on that channel\r\n";
                send(fd, err.c_str(), err.size(), 0);
                return;
            }
            paramIndex++;
            if (targetFD != -1 && chan.hasClient(targetFD))
                chan.setOperator(targetFD, isItPlus);
        }
        else if (c == 'l')
        {
            if (isItPlus)
            {
                if (paramIndex >= params.size())
                {
                    send(fd, "461 MODE :Not enough parameters\r\n", 34, 0);
                    return;
                }

                int limit = std::atoi(params[paramIndex++].c_str());
                if (limit <= 0)
                {
                    send(fd, "461 MODE :Invalid limit\r\n", 27, 0);
                    return;
                }
                chan.setLimit(limit);
            }
            else
                chan.removeLimit();
        }
        else
        {
            std::string err = ":server 472 ";
            err += modes[i];
            err += " :is unknown mode char\r\n";
            send(fd, err.c_str(), err.size(), 0);
        }
    }


    std::string modeMsg = ":" + client.getNick() + " MODE " + chanName +
                      " " + modes;

    for (size_t i = 0; i < paramIndex; i++)
        modeMsg += " " + params[i];

    modeMsg += "\r\n";

    for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
    {
        // if (it->first != fd) // pas envoyer au client qui envoie (pas sur)
        send(it->first, modeMsg.c_str(), modeMsg.size(), 0);
    }

}
