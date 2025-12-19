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
