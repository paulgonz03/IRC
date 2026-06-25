#include "Channel.hpp"

Channel::Channel(std::string name) : _name(name)
{
    /* Generic data */
    _clients.clear();
    _operators.clear();

    /* Modes data */
    _password = "";
    _maxUsers = 0;
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

void Channel::sendMessage(std::string prefix, std::string command, std::string arguments)
{
    for (std::map<std::string, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
        (it->second)->sendMessage(prefix, command, arguments);
}

void Channel::sendMessage(Client *client, std::string prefix, std::string command, std::string arguments)
{
    client->sendMessage(prefix, command, arguments);
}