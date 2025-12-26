#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"
#include <map>

class Channel
{
    private :
                std::string _name;
                std::string _topic;
                std::string _passwordCh;
                bool _inviteOnly; //
                bool _topicRestricted; //
                
                std::string _passwordK;

                std::map<int, bool> _clientCh;
                std::map<int, bool> _invited;

                std::string _key; //
                std::map<int, bool> _clients; //


    public :
                Channel(){};
                Channel(std::string &name): _name(name){}
                ~Channel();

                void addClient(int fd, bool isOper);
                void removeClient(int fd);
                bool hasClient(int fd)const;
                const std::map<int, bool> &getClients()const;
                bool isOperator(int fd) const;
                void setOperator(int fd, bool isOp);

                void setInviteOnly(bool val){ _inviteOnly = val;}
                bool getInvOnly()const {return _inviteOnly;}

                void setTopicRestricted(bool val){_topicRestricted = val;}
                bool getTopicRest(){return _topicRestricted;}

                void setPass(std::string newPass){_passwordK = newPass;}
                bool hasPass() {return !_passwordK.empty();}
                void removePass() {_passwordK.clear();}
                bool checkPass(std::string newPass) { return _passwordK == newPass;}

                const std::string &getTopic() const;
                void setTopic(const std::string &topic);
            std::string getModesString();


};



#endif
