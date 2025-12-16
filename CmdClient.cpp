#include "Server.hpp"


void Server::passCmd(std::string pass, int fd)
{

    if (pass == _password)
    {
        _clients[fd].setPassOK(true);
        std::cout << "✅ Password OK pour fd=" << fd << std::endl;
    }
    else
    {
        _clients[fd].setPassOK(false);
        std::string err = ":server 464 * :Password incorrect\r\n";
        send(fd, err.c_str(), err.size(), 0);
        std::cout << "❌ Mauvais password pour fd=" << fd << std::endl;
    }
}

void Server::nickCmd(std::string nick, int fd)
{
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
            std::cout << "📩 CMD reçue du client fd=" << fd << " : " << line << std::endl;

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
                std::string user, mode, unused;
                ss >> user >> mode >> unused;
                if (!user.empty())
                {
                    _clients[fd].setUser(user);
                    std::cout << "👤 User défini : " << user << " pour fd=" << fd << std::endl;
                }
            }
            Client &client = _clients[fd];
            if (client.isPassOK() && !client.getNick().empty() && !client.getUser().empty() && !client.isRegistered())
            {
                client.setRegistered(true);
                std::cout << "🎉 Client fd=" << fd << " est maintenant ENREGISTRÉ !" << std::endl;

                std::string welcome = ":server 001 " + client.getNick() + " :Welcome to the FT_IRC Network, " + client.getNick() + "\r\n";
                send(fd, welcome.c_str(), welcome.size(), 0);
            }
        }

        // effacer la ligne traitée
        clientBuff.erase(0, pos + 1);
    }
}