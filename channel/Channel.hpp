#pragma once

#include <string>
#include <vector>
#include <map>

class Client;
class Channel
{
    private:
        std::string _name;
        std::map<std::string, Client *> _clients;
        std::map<std::string, Client *> _operators;
        std::map<std::string, Client *> _invit;

        std::string _topic;

        bool _inviteOnly;
        bool _topicRestricted;
        bool _hasKey;
        std::string _key;
        bool _hasLimit;
        size_t _limit;

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
        const std::map<std::string, Client *> &getClients() const;
        bool    isEmpty() const;
        void    renameClient(std::string oldNick, std::string newNick, Client *client);

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

        /* Modes */
        bool   isInviteOnly() const;
        void   setInviteOnly(bool value);
        bool   isTopicRestricted() const;
        void   setTopicRestricted(bool value);
        bool   hasKey() const;
        void   setKey(std::string key);
        void   unsetKey();
        std::string getKey() const;
        bool   hasLimit() const;
        void   setLimit(size_t limit);
        void   unsetLimit();
        size_t getLimit() const;
        std::string getModeString() const;

        void sendMessage(Client *client, std::string prefix, std::string command, std::string arguments);
        void sendMessage(std::string prefix, std::string command, std::string arguments);
        void sendMessageExcept(Client *except, std::string prefix, std::string command, std::string arguments);
};
