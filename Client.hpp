#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int         _fd;
    std::string _nick;
    std::string _user;
    bool        _passOK;
    bool        _registered;
    std::string _buffer;

public:
    Client();
    Client(int fd);

    // Getters
    int getFD() const;
    const std::string &getNick() const;
    const std::string &getUser() const;
    bool isPassOK() const;
    bool isRegistered() const;
    std::string &getBuffer();

    // Setters
    void setNick(const std::string &nick);
    void setUser(const std::string &user);
    void setPassOK(bool value);
    void setRegistered(bool value);
};

#endif
