#include "Channel.hpp"

Channel::Channel(std::string name) : _name(name)
{
    /* Generic data */
    _clients.clear();
    _operators.clear();

    /* Modes data */
    _key = "";
    _maxUsers = 0;
    _topic = "";
    _topicRestricted = false;
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
        if (_key == arg[1])
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
    if (_clients.size() < _maxUsers)
        return (true);
    client->sendMessage(SERVER_PREFIX, ERR_CHANNELISFULL, client->getNickname() + " " + _name + " :Cannot join channel (+i)");
    return (false);
}

bool Channel::canJoin(Client *client, std::vector<std::string> arg)
{
    for (std::vector<t_channel_modes>::iterator it = _modes.begin(); it != _modes.end(); it++)
    {
        if (*it == MODE_K && checkModeK(client, arg) == false)
            return false;
        else if (*it == MODE_I && checkModeI(client) == false)
            return false;
        else if (*it == MODE_L && checkModeL(client) == false)
            return false;
    }
    return (true);
}

void Channel::joinToChannel(Client *client)
{
    addClient(client);
    client->sendJoinMessage(this);
    for(std::vector<t_channel_modes>::iterator it = _modes.begin(); it != _modes.end(); it++)
    {
        if(*it == MODE_I)
            _invit.erase(client->getNickname());
    }
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

bool Channel::isEmpty() const
{
    return (_clients.empty());
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

/**********************/
/* Check invitations */
/**********************/

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

bool Channel::isTopicRestricted() const
{
    return (_topicRestricted);
}

void Channel::setTopicRestricted(bool value)
{
    _topicRestricted = value;
}

/*********/
/* Modes */
/*********/

bool Channel::hasMode(t_channel_modes mode) const
{
    for (size_t i = 0; i < _modes.size(); i++)
        if (_modes[i] == mode)
            return (true);
    return (false);
}

void Channel::setMode(t_channel_modes mode)
{
    if (!hasMode(mode))
        _modes.push_back(mode);
}

void Channel::unsetMode(t_channel_modes mode)
{
    for (std::vector<t_channel_modes>::iterator it = _modes.begin(); it != _modes.end(); it++)
    {
        if (*it == mode)
        {
            _modes.erase(it);
            return;
        }
    }
}

void Channel::setKey(std::string key)
{
    _key = key;
}

void Channel::setLimit(size_t limit)
{
    _maxUsers = limit;
}

std::string Channel::getModeString() const
{
    std::string modes = "+";
    std::string params = "";

    for (std::vector<t_channel_modes>::const_iterator it = _modes.begin(); it != _modes.end(); it++)
    {
        if (*it == MODE_I)
            modes += "i";
        else if (*it == MODE_K)
        {
            modes += "k";
            params += " " + _key;
        }
        else if (*it == MODE_L)
        {
            std::stringstream ss;
            ss << _maxUsers;
            modes += "l";
            params += " " + ss.str();
        }
    }
    if (_topicRestricted)
        modes += "t";
    if (modes == "+")
        return ("+");
    return (modes + params);
}

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