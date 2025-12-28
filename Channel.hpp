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

                std::map<int, bool> _clientCh;
                std::map<int, bool> _invited;

                bool _inviteOnly; //
                bool _topicRestricted; //
                
                std::string _passwordK;
                std::string _key; //
                std::map<int, bool> _clients; //
                int _clientLimit;


    public :
                Channel(): _inviteOnly(false), _topicRestricted(false), _clientLimit(-1){};
                Channel(std::string &name);
                ~Channel();

                void addClient(int fd, bool isOper);
                void removeClient(int fd);
                bool hasClient(int fd)const;

                const std::map<int, bool> &getClients()const;
                bool isOperator(int fd) const;
                void setOperator(int fd, bool isOp);

                void setTopicRestricted(bool val){_topicRestricted = val;}
                bool getTopicRest(){return _topicRestricted;}

                void setPass(std::string newPass){_passwordK = newPass;}
                bool hasPass() {return !_passwordK.empty();}
                void removePass() {_passwordK.clear();}
                bool checkPass(std::string newPass) { return _passwordK == newPass;}

                const std::string &getTopic() const;
                void setTopic(const std::string &topic);
                std::string getModesString();

                void inviteClient(int fd);
                void removeInvite(int fd);
                bool isInvited(int fd);
                bool getInviteOnly();
                void setInviteOnly(bool val);

                void setLimit(int limit);
                void removeLimit();
                bool hasLimit();
                int getLimit();
};



#endif
