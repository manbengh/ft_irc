#include "Channel.hpp"

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
        _invited.erase(fd);
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


const std::string &Channel::getTopic() const
{
    return _topic;
}

void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
}

void Channel::inviteClient(int fd)
{
    _invited[fd] = true;
}

void Channel::removeInvite(int fd)
{
    _invited.erase(fd);
}


bool Channel::isInvited(int fd)
{
    return(_invited.find(fd) != _invited.end());
}
bool Channel::inviteOnly()
{
    return (_inviteOnly);
}

void Channel::setInviteIsOk(bool value)
{
    _inviteOnly = value;
}