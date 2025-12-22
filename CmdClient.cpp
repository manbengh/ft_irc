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
        bool nickInUse = false;
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->first == fd) // ignorer le client courant
                continue;
            if (it->second.getNick() == nick && !it->second.getNick().empty())
            {
                nickInUse = true;
                break;
            }
        }

        if (nickInUse)
        {
            std::string err = ":server 433 " + nick + " :Nickname is already in use\r\n";
            send(fd, err.c_str(), err.size(), 0);
            std::cout << "⚠️  Nick " << nick << " déjà utilisé — NICK ignoré pour fd=" << fd << std::endl;
        }
        else
        {
            _clients[fd].setNick(nick);
            std::cout << "📛 Nick défini : " << nick << " pour fd=" << fd << std::endl;
        }
    }
}





void Server::handleJoin(int fd, std::string chanName)
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
        std::string err = ":server 461 JOIN :Not enough parameters\r\n";
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

    if (!chan.hasClient(fd))//verifie si le client est deja dans le channel
    {
        bool first = chan.getClients().empty();//premier client = operateur
        chan.addClient(fd, first);//ajoute le client
    }
    std::string joinMsg = client.getNick() + " JOIN " + chanName + "\r\n";
    for (std::map<int , bool>::const_iterator it = chan.getClients().begin();
        it != chan.getClients().end(); it++)
        {
            send(it->first, joinMsg.c_str(), joinMsg.size(), 0);
        }
    std::cout << "🟢 Client fd=" << fd << " a rejoint " << chanName << std::endl;

}



void Server::handlePart(int fd, std::string chanName, std::string reason)
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

    std::string fullMsg;
    if (!reason.empty())
        fullMsg = ":" + parter.getNick() + " PART " + chanName + " :" + reason + "\r\n";
    else
        fullMsg = ":" + parter.getNick() + " PART " + chanName + "\r\n";
    for (std::map<int,bool>::const_iterator it = chan.getClients().begin();
             it != chan.getClients().end(); ++it)
    {
        // if (it->first != fd) // pas envoyer au client qui envoie (pas sur)
        send(it->first, fullMsg.c_str(), fullMsg.size(), 0);
    }
    chan.removeClient(fd);

    if (chan.getClients().empty())
        _channels.erase(chanName);
}




void Server::handlePrivMsg(int fd, std::string target, std::string msg)
{
    Client &sender = _clients[fd];

    if (!sender.isRegistered())
    {
        send(fd, "451 :You have not registered\r\n", 31, 0);
        return;
    }

    // if (target.empty() || msg.empty())
    //     return;

    if (target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
        {
            std::string err = ":server 403 " + target + " :No such channel\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        Channel &chan = _channels[target];
        std::string fullMsg =   ":" + sender.getNick() +
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

        std::string fullMsg =   ":" + sender.getNick() +
                                " PRIVMSG " + target + " :" + msg + "\r\n";
        send(targetFd, fullMsg.c_str(), fullMsg.size(), 0);
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
            std::cout << "📩 CMD reçue du client fd=" << fd << " : " << line << "." << std::endl;

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
                std::string chanName;
                ss >> chanName;
                handleJoin(fd, chanName);
            }
            else if (cmd == "PRIVMSG")
            {
                std::string target;
                ss >> target;

                if (target.empty())
                {
                    send(fd, ":server 411 :No recipient given (PRIVMSG)\r\n", 44, 0);
                    // clientBuff.erase(0, pos + 1);
                    continue ;
                }

                std::string rest;
                std::getline(ss, rest);
                while (!rest.empty() && rest[0] == ' ')
                    rest.erase(0, 1);

                size_t colonPos = rest.find(':');//chercher le ':' qui introduit le msg
                if (colonPos == std::string::npos)
                {
                    send(fd, ":server 412 :No text to send\r\n", 33, 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }

                std::string msg = rest.substr(colonPos + 1);// tout après le ':'
                if (msg.empty())
                {
                    send(fd, ":server 412 :No text to send\r\n", 33, 0);
                    clientBuff.erase(0, pos + 1);
                    continue ;
                }

                handlePrivMsg(fd, target, msg);
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
                handlePart(fd, chanName, reason);
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



            else
            {
                std::string err = ":server 421 " + cmd + " :Unknown command\r\n";
                send(fd, err.c_str(), err.size(), 0);
            }

            Client &client = _clients[fd];
            if (client.isPassOK() && !client.getNick().empty() && !client.getUser().empty() && !client.isRegistered())
            {
                client.setRegistered(true);
                std::cout << "🎉 Client fd=" << fd << " is now REGISTERED !" << std::endl;

                std::string welcome = ":server 001 " + client.getNick() + " :Welcome to the FT_IRC Network, " + client.getNick() + "\r\n";
                send(fd, welcome.c_str(), welcome.size(), 0);
            }
        }

        clientBuff.erase(0, pos + 1);// effacer la ligne traitée
    }
}
