#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"


class Channel
{
    private :
                std::string _name;
                std::string _topic;
                std::map<int, bool> _clientCh;
                std::map<int, Client> _clientChInv;
                std::string _passwordCh;
                bool _ifPass;
                bool _isInvited;
                bool _registeredCH;

    public :

                Channel();
                Channel(std::string &name): _name(name){}
                ~Channel();

        
                bool IfPassOkCH() const;
                bool IsRegisteredCH() const;


                void setIfPassOK(bool value);
                void setRegisteredCH(bool value);

};




#endif
