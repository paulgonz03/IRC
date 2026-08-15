#pragma once

#include <string>
#include <vector>
#include <map>
#include "../client/Client.hpp"
#include "../irc_codes.hpp"

class Client;
class Channel
{
    private:
        std::string _name;
        std::map<std::string, Client *> _clients;
        std::map<std::string, Client *> _operators;

        std::vector<t_channel_modes> _modes;
        std::string _key;
        std::map<std::string, Client *> _invit;
        size_t _maxUsers;
        std::string _topic;
        bool _topicRestricted;

        static std::map<std::string, Channel *> _channels;

    public:
        Channel( std::string name );
        ~Channel();

        /* Static methods/interact with the global data */
        static Channel *addChannel(std::string name);
        static void deleteChannel(std::string name);
        static void deleteChannels();
        static Channel *findChannel(std::string name);
        
        /**/
        std::string getChannelName();
        bool checkModeK(Client *Client, std::vector<std::string> arg);
        bool checkModeI(Client *client);
        bool checkModeL(Client *client);
        bool canJoin(Client *client, std::vector<std::string> arg);
        void joinToChannel(Client *client);

        /* Check users */
        void    addClient(Client *client);
        void    removeClient(Client *client);
        Client* hasClient(std::string name);
        bool    hasClient(Client *client);
        std::string getAllClients();
        bool    isEmpty() const;

        /* Check operators */
        void    addOperator(Client* oper);
        void    removeOperator(Client *oper);
        Client* isOperator(std::string name);
        bool    isOperator(Client* oper);

        /* Check invitations */
        void    addInvite(Client *client);

        /* Topic */
        std::string getTopic() const;
        void        setTopic(std::string topic);
        bool        isTopicRestricted() const;
        void        setTopicRestricted(bool value);

        /* Modes */
        bool        hasMode(t_channel_modes mode) const;
        void        setMode(t_channel_modes mode);
        void        unsetMode(t_channel_modes mode);
        void        setKey(std::string key);
        void        setLimit(size_t limit);
        std::string getModeString() const;

        void sendMessage(Client *client, std::string prefix, std::string command, std::string arguments);
        void sendMessage(std::string prefix, std::string command, std::string arguments);
        void sendMessageExcept(Client *except, std::string prefix, std::string command, std::string arguments);
};
