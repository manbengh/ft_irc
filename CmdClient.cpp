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