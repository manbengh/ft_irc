#include "Channel.hpp"

// Channel();
Channel::~Channel(){}

void Channel::addClient(int fd, bool isOper)
{
    _clientCh[fd] = isOper;
}

void Channel::removeClient(int fd)
{
    bool ope = false;
    std::map<int, bool>::iterator it = _clientCh.find(fd);

    if (it != _clientCh.end())
    {
        ope = it->second;
        _clientCh.erase(it);
    }

    if (ope && !_clientCh.empty())
        _clientCh.begin()->second = true;
}

bool Channel::hasClient(int fd)const
{
    return _clientCh.find(fd) != _clientCh.end();
}

const std::map<int, bool> &Channel::getClients()const
{
    return _clientCh;
}


