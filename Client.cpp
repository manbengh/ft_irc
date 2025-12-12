#include "Client.hpp"

Client::Client()
    : _fd(-1), _passOK(false), _registered(false)
{
}

Client::Client(int fd)
    : _fd(fd), _passOK(false), _registered(false)
{
}

// Getters

int Client::getFD() const 
{
    return _fd;
}

const std::string &Client::getNick() const 
{
    return _nick;
}

const std::string &Client::getUser() const 
{
    return _user;
}


bool Client::isPassOK() const 
{
    return _passOK;
}

bool Client::isRegistered() const 
{
    return _registered;
}

std::string &Client::getBuffer() 
{
    return _buffer;
}

// Setters

void Client::setNick(const std::string &nick) 
{
    _nick = nick;
}

void Client::setUser(const std::string &user) 
{
    _user = user;
}

void Client::setPassOK(bool value) 
{
    _passOK = value;
}

void Client::setRegistered(bool value) 
{
    _registered = value;
}
