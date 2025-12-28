#include "Server.hpp"


void Server::passCmd(std::string pass, int fd)
{

    if (pass.empty())
    {
        std::string err = ":server 461 PASS :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    if (pass == _password)
    {
        _clients[fd].setPassOK(true);
        std::cout << "✅ Password OK pour fd=" << fd << std::endl;
    }
    else
    {
        _clients[fd].setPassOK(false);
        std::string err = ":server 464 :Password incorrect\r\n";
        send(fd, err.c_str(), err.size(), 0);
        std::cout << "❌ Mauvais password pour fd=" << fd << std::endl;
    }
}


void Server::nickCmd(std::string nick, int fd)
{
    if (nick.empty())
    {
        std::string err = ":server 431 :No nickname given\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    if (!nick.empty())
    {
        bool nickUse = false;
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->first == fd) // ignorer le client courant
                continue;
            if (it->second.getNick() == nick && !it->second.getNick().empty())
            {
                nickUse = true;
                break;
            }
        }
        if (nickUse)
        {
            std::string err = ":server 433 " + (_clients[fd].getNick().empty() ? "*" : _clients[fd].getNick()) +
                    " " + nick + " :Nickname is already in use\r\n";
            send(fd, err.c_str(), err.size(), 0);
            std::cout << "⚠️  Nick " << nick << " deja utilise — NICK ignore pour fd=" << fd << std::endl;
            return;
        }
        else
        {
            _clients[fd].setNick(nick);
            std::cout << "🆔 Nick défini : " << nick << " pour fd=" << fd << std::endl;
        }
    }
}


void Server::ftJoin(int fd, std::string chanName, std::string key)
{
    Client &client = _clients[fd];

    if (!client.isRegistered())
    {
        std::string err = "451 :You have not registered\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return ;
    }

    if (chanName.empty())
    {
        std::string err = ":server 461 JOIN :No t enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    if (chanName[0] != '#' || chanName.size() == 1)
    {
        std::string err = ":server 403 " + chanName + " :No such channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    
    if (_channels.find(chanName) == _channels.end())
        _channels[chanName] = Channel(chanName);
    
    Channel &chan = _channels[chanName];

    if(chan.getInviteOnly() && !chan.isInvited(fd))
    {
        std::string err = ":server 473 " + client.getNick() + " " + chanName +
                        " :Cannot join channel (+i)\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    if (chan.hasPass() && !chan.checkPass(key))
    {
        std::string err = ":server 475 " + client.getNick() + " " + chanName + " :Cannot join channel (+k)\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    if (chan.hasLimit() && chan.getClients().size() >= (size_t)chan.getLimit())
    {
        std::string err = ":server 471 " + client.getNick() + " " + chanName + " :Cannot join channel (+l)\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    if (chan.hasClient(fd))
    {
        std::string err = ":server 443 " + client.getNick() + " " + client.getNick() 
                        + " " + chanName + " :User is already on the channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    if(!chan.hasClient(fd))//verifie si le client est deja dans le channel
    {
        bool first = chan.getClients().empty();//premier client = operateur
        chan.addClient(fd, first);
        chan.removeInvite(fd);
    }

    std::string joinMsg = ":" + client.getNick() + "!" + client.getUser() + "@localhost JOIN " +
                      chanName + "\r\n";
    for (std::map<int , bool>::const_iterator it = chan.getClients().begin();
        it != chan.getClients().end(); it++)
        {
            send(it->first, joinMsg.c_str(), joinMsg.size(), 0);
        }
    std::cout << "🟢 Client fd=" << fd << " a rejoint " << chanName << std::endl;

}



void Server::ftPart(int fd, std::string chanName, std::string reason)
{
    Client &parter = _clients[fd];

    if (!parter.isRegistered())
    {
        send(fd, "451 :You have not registered\r\n", 31, 0);
        return;
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

    std::string allMsg;
    if (!reason.empty())
        allMsg = ":" + parter.getNick() + " PART " + chanName + " :" + reason + "\r\n";
    else
        allMsg = ":" + parter.getNick() + " PART " + chanName + "\r\n";
    for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
    {
        send(it->first, allMsg.c_str(), allMsg.size(), 0);
    }
    chan.removeClient(fd);

    if (chan.getClients().empty())
        _channels.erase(chanName);
}



void Server::ftPrivMsg(int fd, std::string target, std::string msg)
{
    Client &client = _clients[fd];

    if (!client.isRegistered())
    {
        send(fd, "451 :You have not registered\r\n", 31, 0);
        return;
    }
    if (target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
        {
            std::string err = ":server 403 " + target + " :No such channel\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        Channel &chan = _channels[target];

        if (!chan.hasClient(fd))
        {
            std::string err = ":server 404 " + target + " :Cannot send to channel\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }
        std::string allMsg =   ":" + client.getNick() +
                                " PRIVMSG " + target + " :" + msg + "\r\n";

        for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
        {
            if (it->first != fd) // pas envoyer au client qui envoie
                send(it->first, allMsg.c_str(), allMsg.size(), 0);
        }
    }
    // Message privé à un client
    else
    {
        // Cherche le fd du client cible
        int targetFd = -1;
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second.getNick() == target)
            {
                targetFd = it->first;
                break;
            }
        }

        if (targetFd == -1)
        {
            std::string err = ":server 401 " + target + " :No such nick\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        std::string allMsg =   ":" + client.getNick() +
                                " PRIVMSG " + target + " :" + msg + "\r\n";
        send(targetFd, allMsg.c_str(), allMsg.size(), 0);
    }
}


void Server::ftInvite(int fd, std::string &name, std::string &chanName)
{
    Client &invite = _clients[fd];
    
    if (!invite.isRegistered())
    {
        send(fd, "451 :You have not registered\r\n", 31, 0);
        return;
    }
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

    if(!chan.isOperator(fd))
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
        std::string err = ":server 401 " + name + " :No such nick\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    
    if(chan.hasClient(targetFd))
    {
        std::string err = ":server 443 " + invite.getNick() + " " + name + " " + chanName + " :is already on channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    chan.inviteClient(targetFd);

    std::string inviteMsg = ":" + invite.getNick() + " INVITE " + name + " " + chanName + "\r\n";
    send(targetFd, inviteMsg.c_str(), inviteMsg.size(), 0);

    // Confirmation pour l’inviteur
    std::string msgInviting = ":server 341 " + invite.getNick() + " " + name + " " + chanName + "\r\n";
    send(fd, msgInviting.c_str(), msgInviting.size(), 0);
}


void Server::cmdIdentify(std::string &clientBuff, int fd)
{
    size_t pos;
    
    while ((pos = clientBuff.find('\n')) != std::string::npos)
    {
        std::string line = clientBuff.substr(0, pos);

        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (!line.empty())
        {
            std::cout << "📩 CMD reçue du client fd=" << fd << " : [" << line << "]" << std::endl;

            std::stringstream ss(line);
            std::string cmd;
            ss >> cmd;

            
            if (cmd == "PASS")
            {
                std::string pass;
                ss >> pass;
                passCmd(pass, fd);
            }
            else if (cmd == "NICK" && _clients[fd].isPassOK() == true) // nick doit etre unique a chaque clicli pas de double
            {
                std::string nick;
                ss >> nick;
                nickCmd(nick, fd);
            }
            else if (cmd == "USER" && _clients[fd].isPassOK() == true && !_clients[fd].getNick().empty())
            {
                std::string user;
                ss >> user;
                if (!user.empty())
                {
                    _clients[fd].setUser(user);
                    std::cout << "👤 User defini : " << user << " pour fd=" << fd << std::endl;
                }
            }
            else if (cmd == "JOIN")
            {
                std::string chanName;
                std::string key;
                ss >> chanName;
                ss >> key;
                ftJoin(fd, chanName, key);
            }
            else if (cmd == "PRIVMSG")
            {
                std::string target;
                ss >> target;

                if (target.empty())
                {
                    send(fd, ":server 411 :No recipient given (PRIVMSG)\r\n", 44, 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }
                std::string rest;
                std::getline(ss, rest);
                while(!rest.empty() && rest[0] == ' ')
                    rest.erase(0, 1);

                size_t colonPos = rest.find(':');//chercher le ':' qui introduit le msg
                if (colonPos == std::string::npos)
                {
                    std::string err = ":server 412 :No text to send\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }
                std::string msg = rest.substr(colonPos + 1);// tout après le ':'
                if (msg.empty())
                {
                    std::string err = ":server 412 :No text to send\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }
                ftPrivMsg(fd, target, msg);
            }
            else if (cmd == "PART")
            {
                std::string chanName;
                ss >> chanName;
            
                std::string reason;
                std::getline(ss, reason);

                if (!reason.empty() && reason[0] == ' ')
                    reason.erase(0, 1);
                if (!reason.empty() && reason[0] == ':')
                    reason.erase(0, 1);
                ftPart(fd, chanName, reason);
            }
            else if (cmd == "TOPIC")
            {
                std::string chanName;
                ss >> chanName;

                std::string remains;
                std::getline(ss, remains);

                while (!remains.empty() && remains[0] == ' ')
                    remains.erase(0, 1);
                std::string topic;
                if (!remains.empty() && remains[0] == ':')
                {
                    remains.erase(0, 1);    // pour enlever les : qui restaient 
                    topic = remains.substr(1);
                }
                ftTopic(fd, chanName, remains);
            }
            else if (cmd == "PING")
            {
                std::string line;
                ss >> line;

                if (line.empty())
                    line = "server";

                std::string result = "PONG " + line + "\r\n";
                send(fd, result.c_str(), result.size(), 0);
            }
            else if(cmd == "INVITE")
            {
                std::string invite;
                ss >> invite;

                std::string chanName;
                ss >> chanName;

                if(invite.empty() || chanName.empty())
                {
                    std::string err = ":server 461 INVITE :Not enough parameters\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }
                ftInvite(fd, invite, chanName);
            }
            else if(cmd == "MODE")
            {
                std::string chanName;
                ss >> chanName;

                std::string modes;
                ss >> modes;

                std::vector<std::string> params;
                std::string p;
                while (ss >> p)
                    params.push_back(p);

                if(modes.empty() || chanName.empty())
                {
                    std::string err = ":server 461 MODE :Not enough parameters\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }
                ftMode(fd, chanName, modes, params);
            }
            else if(cmd == "KICK")
            {
                std::string chanName;
                ss >> chanName;

                std::string nick;
                ss >> nick;

                if(nick.empty() || chanName.empty())
                {
                    std::string err = ":server 461 KICK :Not enough parameters\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }
                ftKick(fd, nick, chanName);
            }

            Client &client = _clients[fd];
            if (client.isPassOK() && !client.getNick().empty() && !client.getUser().empty() && !client.isRegistered())
            {
                client.setRegistered(true);
                std::cout << "🎉 Client fd=" << fd << " is now REGISTERED !" << std::endl;
                
                std::string welcome = ":server 001 " + client.getNick() + " :Welcome to the FT_IRC Network, " 
                    + client.getNick() + "!" + client.getUser() + "@localhost\r\n";
                send(fd, welcome.c_str(), welcome.size(), 0);
            }
        }
        clientBuff.erase(0, pos + 1);// effacer la ligne traitée
    }
}
