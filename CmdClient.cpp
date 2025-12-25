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
            std::string err = ":server 433 " + nick + " :Nickname is already in use\r\n";
            send(fd, err.c_str(), err.size(), 0);
            std::cout << "⚠️  Nick " << nick << " déjà utilisé — NICK ignoré pour fd=" << fd << std::endl;
        }
        else
        {
            _clients[fd].setNick(nick);
            std::cout << "✅ Nick défini : " << nick << " pour fd=" << fd << std::endl;
        }
    }
}



void Server::handleJoin(int fd, std::string channelName)
{
    Client &client = _clients[fd];
    if (!client.isRegistered())
    {
        std::string err = "451 :You have not registered\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return ;
    }

    if (channelName.empty())
    {
        std::string err = ":server 461 JOIN :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    if (channelName[0] != '#' || channelName.size() == 1)
    {
        std::string err = ":server 403 " + channelName + " :No such channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    
    if (_channels.find(channelName) == _channels.end())
        _channels[channelName] = Channel(channelName);
    
    Channel &chan = _channels[channelName];

    if (!chan.hasClient(fd))//verifie si le client est deja dans le channel
    {
        bool first = chan.getClients().empty();//premier client = operateur
        chan.addClient(fd, first);//ajoute le client
    }
    std::string joinMsg = client.getNick() + " JOIN " + channelName + "\r\n";
    for (std::map<int , bool>::const_iterator it = chan.getClients().begin();
        it != chan.getClients().end(); it++)
        {
            send(it->first, joinMsg.c_str(), joinMsg.size(), 0);
        }
    std::cout << "🟢 Client fd=" << fd << " a rejoint " << channelName << std::endl;

}



void Server::handlePart(int fd, std::string channelName, std::string reason)
{
    Client &parter = _clients[fd];

    if (!parter.isRegistered())
    {
        send(fd, "451 :You have not registered\r\n", 31, 0);
        return;
    }
    if (channelName.empty() || channelName.find('#'))
        return;
        
    if (_channels.find(channelName) == _channels.end())
    {
        std::string err = ":server 403 " + channelName + " :No such channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    
    Channel &chan = _channels[channelName];

    if (!chan.hasClient(fd))
    {
        std::string err = ":server 442 " + channelName + " :You're not on that channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    std::string fullMsg;
    if (!reason.empty())
        fullMsg = ":" + parter.getNick() + " PART " + channelName + " :" + reason + "\r\n";
    else
        fullMsg = ":" + parter.getNick() + " PART " + channelName + "\r\n";
    for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
    {
        send(it->first, fullMsg.c_str(), fullMsg.size(), 0);
    }
    chan.removeClient(fd);

    if (chan.getClients().empty())
        _channels.erase(channelName);
}



void Server::handlePrivMsg(int fd, std::string target, std::string msg)
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
        std::string fullMsg =   ":" + client.getNick() +
                                " PRIVMSG " + target + " :" + msg + "\r\n";

        for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
        {
            if (it->first != fd) // pas envoyer au client qui envoie
                send(it->first, fullMsg.c_str(), fullMsg.size(), 0);
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

        std::string fullMsg =   ":" + client.getNick() +
                                " PRIVMSG " + target + " :" + msg + "\r\n";
        send(targetFd, fullMsg.c_str(), fullMsg.size(), 0);
    }
}


void Server::InviteInchan(int fd, std::string &name, std::string &channelName)
{
    Client &invite = _clients[fd];
    
    if (!invite.isRegistered())
    {
        send(fd, "451 :You have not registered\r\n", 31, 0);
        return;
    }


}


void Server::cmdIdentify(std::string &clientBuff, int fd)
{
    size_t pos;
    
    while ((pos = clientBuff.find('\n')) != std::string::npos)
    {
        std::string line = clientBuff.substr(0, pos);
        // enlever '\r' final si present
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
                    std::cout << "👤 User défini : " << user << " pour fd=" << fd << std::endl;
                }
            }
            else if (cmd == "JOIN")
            {
                std::string channelName;
                ss >> channelName;
                handleJoin(fd, channelName);
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
                handlePrivMsg(fd, target, msg);
            }
            else if (cmd == "PART")
            {
                std::string channelName;
                ss >> channelName;
            
                std::string reason;
                std::getline(ss, reason);

                if (!reason.empty() && reason[0] == ' ')
                    reason.erase(0, 1);
                if (!reason.empty() && reason[0] == ':')
                    reason.erase(0, 1);
                handlePart(fd, channelName, reason);
            }
            // else if (cmd == "QUIT")
            // {
            //     std::string reason;
            //     ss >> reason;

            //     if (!reason.empty() && reason[0] == ' ')
            //         reason.erase(0, 1);
            //     if (!reason.empty() && reason[0] == ':')
            //         reason.erase(0, 1);
            //     if (reason.empty())
            //         reason = "Client Quit";
            //     // handleQuit(fd, reason);
            // }
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

                std::string channelName;
                ss >> channelName;

                if(invite.empty() || channelName.empty())
                {
                    std::string err = ":server 461 INVITE :Not enough parameters\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }
                InviteInchan(fd, invite, channelName);
            }

            Client &client = _clients[fd];
            if (client.isPassOK() && !client.getNick().empty() && !client.getUser().empty() && !client.isRegistered())
            {
                client.setRegistered(true);
                std::cout << "🎉 Client fd=" << fd << " is now REGISTERED !" << std::endl;
                //   "<client> :Welcome to the <networkname> Network, <nick>[!<user>@<host>]"
                std::string welcome = ":server 001 " + client.getNick() + " :Welcome to the FT_IRC Network, " 
                    + client.getNick() + "!" + client.getUser() + "@localhost\r\n";
                send(fd, welcome.c_str(), welcome.size(), 0);
            }
        }
        clientBuff.erase(0, pos + 1);// effacer la ligne traitée
    }
}
