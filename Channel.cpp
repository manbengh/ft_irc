#include "Channel.hpp"

// Channel();
Channel::~Channel(){}

void Channel::addClient(int fd, bool isOper)
{
    _clientCh[fd] = isOper;
}

void Channel::removeClient(int fd)
{
    _clientCh.erase(fd);
}

bool Channel::hasClient(int fd)const
{
    return _clientCh.find(fd) != _clientCh.end();
}

const std::map<int, bool> &Channel::getClients()const
{
    return _clientCh;
}


