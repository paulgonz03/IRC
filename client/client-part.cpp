#include "Client.hpp"

void Client::addChannel(Channel *channel)
{
    for (size_t i = 0; i < _channels.size(); i++)
        if (_channels[i] == channel)
            return;
    _channels.push_back(channel);
}

void Client::removeChannelRef(Channel *channel)
{
    for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (*it == channel)
        {
            _channels.erase(it);
            return;
        }
    }
}

void Client::leaveAllChannels(std::string reason)
{
    std::map<std::string, Client *> notified;

    for (size_t i = 0; i < _channels.size(); i++)
    {
        Channel *channel = _channels[i];
        const std::map<std::string, Client *> &members = channel->getClients();
        for (std::map<std::string, Client *>::const_iterator it = members.begin(); it != members.end(); it++)
            if (it->second != this)
                notified[it->first] = it->second;
        channel->removeClient(this);
        channel->removeOperator(this);
        if (channel->isEmpty())
            Channel::deleteChannel(channel->getChannelName());
    }
    _channels.clear();

    std::string prefix = getIdentity();
    for (std::map<std::string, Client *>::iterator it = notified.begin(); it != notified.end(); it++)
        it->second->sendMessage(prefix, "QUIT", ":" + reason);
}

void Client::part_command(std::vector<std::string> args)
{
    if (!_is_authenticated)
    {
        sendMessage(SERVER_PREFIX, NOT_REGISTERED, "* :You have not registered");
        return;
    }
    if (args.size() < 1)
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* PART :Not enough parameters");
        return;
    }

    std::string reason = args.size() > 1 ? args[1] : _nickname;
    std::stringstream ss(args[0]);
    std::string chanName;
    while (std::getline(ss, chanName, ','))
    {
        if (chanName.empty())
            continue;
        Channel *channel = Channel::findChannel(chanName);
        if (!channel)
        {
            sendMessage(SERVER_PREFIX, ERR_NOSUCHCHANNEL, _nickname + " " + chanName + " :No such channel");
            continue;
        }
        if (!channel->hasClient(this))
        {
            sendMessage(SERVER_PREFIX, ERR_NOTONCHANNEL, _nickname + " " + chanName + " :You're not on that channel");
            continue;
        }
        channel->sendMessage(getIdentity(), "PART", chanName + " :" + reason);
        channel->removeClient(this);
        channel->removeOperator(this);
        removeChannelRef(channel);
        if (channel->isEmpty())
            Channel::deleteChannel(chanName);
    }
}

void Client::quit_command(std::vector<std::string> args)
{
    std::string reason = args.size() > 0 ? args[0] : "Client Quit";

    leaveAllChannels(reason);
    sendRaw("ERROR :Closing Link: " + _hostname + " (" + reason + ")");
    _quit = true;
}
