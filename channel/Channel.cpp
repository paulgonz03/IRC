#include "Channel.hpp"
#include "../client/Client.hpp"
#include <sstream>

Channel::Channel(std::string name) : _name(name)
{
    /* Generic data */
    _clients.clear();
    _operators.clear();

    /* Modes data */
    _topic = "";
    _inviteOnly = false;
    _topicRestricted = false;
    _hasPassword = false;
    _password = "";
    _hasLimit = false;
    _limit = 0;
}

Channel::~Channel()
{
}

std::string Channel::getChannelName()
{
    return (_name);
}

bool Channel::checkModeK(Client *client, std::vector<std::string> arg)
{
    if (arg.size() > 1)
        if (_password == arg[1])
            return (true);
    client->sendMessage(SERVER_PREFIX, ERR_BADCHANNELKEY, client->getNickname() + " " + _name + " :Cannot join channel (+k)");
    return (false);
}

bool Channel::checkModeI(Client *client)
{
    if (_invit.find(client->getNickname()) != _invit.end())
        return (true);
    client->sendMessage(SERVER_PREFIX, ERR_INVITEONLYCHAN, client->getNickname() + " " + _name + " :Cannot join channel (+i)");
    return (false);
}

bool Channel::checkModeL(Client *client)
{
    if (_clients.size() < _limit)
        return (true);
    client->sendMessage(SERVER_PREFIX, ERR_CHANNELISFULL, client->getNickname() + " " + _name + " :Cannot join channel (+l)");
    return (false);
}

bool Channel::canJoin(Client *client, std::vector<std::string> arg)
{
    if (_hasPassword && checkModeK(client, arg) == false)
        return false;
    if (_inviteOnly && checkModeI(client) == false)
        return false;
    if (_hasLimit && checkModeL(client) == false)
        return false;
    return (true);
}

void Channel::joinToChannel(Client *client)
{
    addClient(client);
    client->addChannel(this);
    client->sendJoinMessage(this);
    _invit.erase(client->getNickname());
}

/***************/
/* Check users */
/***************/

void Channel::addClient(Client *client)
{
    std::string clientNickname = client->getNickname();
    std::map<std::string, Client *>::iterator it = _clients.find(clientNickname);
    if (it == _clients.end())
        _clients[clientNickname] = client;
}

void Channel::removeClient(Client *client)
{
    std::string clientNickname = client->getNickname();
    std::map<std::string, Client *>::iterator it = _clients.find(clientNickname);
    if (it != _clients.end())
        _clients.erase(it);
}

Client *Channel::hasClient(std::string name)
{
    std::map<std::string, Client *>::iterator it = _clients.find(name);
    if (it != _clients.end())
        return it->second;
    return NULL;
}

bool Channel::hasClient(Client *client)
{
    std::string clientNickname = client->getNickname();
    std::map<std::string, Client *>::iterator it = _clients.find(clientNickname);
    if (it != _clients.end())
        return true;
    return false;
}

std::string Channel::getAllClients()
{
    std::string clients = "";
    for (std::map<std::string, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (_operators.find(it->first) != _operators.end())
        {
            if (it == _clients.begin())
                clients = "@" + it->first;
            else
                clients += " @" + it->first;
        }
        else
        {
            if (it == _clients.begin())
                clients = it->first;
            else
                clients += " " + it->first;
        }
    }
    return (clients);
}

const std::map<std::string, Client *> &Channel::getClients() const
{
    return (_clients);
}

bool Channel::isEmpty() const
{
    return (_clients.empty());
}

void Channel::renameClient(std::string oldNick, std::string newNick, Client *client)
{
    std::map<std::string, Client *>::iterator it = _clients.find(oldNick);
    if (it != _clients.end())
    {
        _clients.erase(it);
        _clients[newNick] = client;
    }
    it = _operators.find(oldNick);
    if (it != _operators.end())
    {
        _operators.erase(it);
        _operators[newNick] = client;
    }
    it = _invit.find(oldNick);
    if (it != _invit.end())
    {
        _invit.erase(it);
        _invit[newNick] = client;
    }
}

/******************/
/* Check operator */
/******************/

void Channel::addOperator(Client *oper)
{
    std::string operNickname = oper->getNickname();
    std::map<std::string, Client *>::iterator it = _operators.find(operNickname);
    if (it == _operators.end())
        _operators[operNickname] = oper;
}

void Channel::removeOperator(Client *oper)
{
    std::string operNickname = oper->getNickname();
    std::map<std::string, Client *>::iterator it = _operators.find(operNickname);
    if (it != _operators.end())
        _operators.erase(it);
}

Client *Channel::isOperator(std::string name)
{
    std::map<std::string, Client *>::iterator it = _operators.find(name);
    if (it != _operators.end())
        return it->second;
    return NULL;
}

bool Channel::isOperator(Client *oper)
{
    std::string operNickname = oper->getNickname();
    std::map<std::string, Client *>::iterator it = _operators.find(operNickname);
    if (it != _operators.end())
        return true;
    return false;
}

/**********/
/* Invite */
/**********/

void Channel::addInvite(Client *client)
{
    _invit[client->getNickname()] = client;
}

/*********/
/* Topic */
/*********/

std::string Channel::getTopic() const
{
    return (_topic);
}

void Channel::setTopic(std::string topic)
{
    _topic = topic;
}

/*********/
/* Modes */
/*********/

bool Channel::isInviteOnly() const
{
    return (_inviteOnly);
}

void Channel::setInviteOnly(bool value)
{
    _inviteOnly = value;
}

bool Channel::isTopicRestricted() const
{
    return (_topicRestricted);
}

void Channel::setTopicRestricted(bool value)
{
    _topicRestricted = value;
}

void Channel::setPassword(std::string password)
{
    _hasPassword = true;
    _password = password;
}

void Channel::unsetPassword()
{
    _hasPassword = false;
    _password = "";
}

void Channel::setLimit(size_t limit)
{
    _hasLimit = true;
    _limit = limit;
}

void Channel::unsetLimit()
{
    _hasLimit = false;
    _limit = 0;
}

std::string Channel::getModeString() const
{
    std::string modes = "+";
    std::string params = "";

    if (_inviteOnly)
        modes += "i";
    if (_topicRestricted)
        modes += "t";
    if (_hasPassword)
    {
        modes += "k";
        params += " " + _password;
    }
    if (_hasLimit)
    {
        std::stringstream ss;
        ss << _limit;
        modes += "l";
        params += " " + ss.str();
    }
    return (modes + params);
}

/************/
/* Messages */
/************/

void Channel::sendMessage(std::string prefix, std::string command, std::string arguments)
{
    for (std::map<std::string, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
        (it->second)->sendMessage(prefix, command, arguments);
}

void Channel::sendMessage(Client *client, std::string prefix, std::string command, std::string arguments)
{
    client->sendMessage(prefix, command, arguments);
}

void Channel::sendMessageExcept(Client *except, std::string prefix, std::string command, std::string arguments)
{
    for (std::map<std::string, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
        if (it->second != except)
            (it->second)->sendMessage(prefix, command, arguments);
}
