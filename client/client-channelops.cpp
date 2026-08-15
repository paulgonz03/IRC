#include "Client.hpp"

void Client::kick_command(std::vector<std::string> args)
{
    if (!_is_authenticated)
    {
        sendMessage(SERVER_PREFIX, NOT_REGISTERED, "* :You have not registered");
        return;
    }
    if (args.size() < 2)
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* KICK :Not enough parameters");
        return;
    }
    Channel *channel = Channel::findChannel(args[0]);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, INVALID_NAME, _nickname + " " + args[0] + " :No such channel");
        return;
    }
    if (channel->hasClient(this) == false)
    {
        sendMessage(SERVER_PREFIX, ERR_NOTONCHANNEL, _nickname + " " + args[0] + " :You're not on that channel");
        return;
    }
    if (channel->isOperator(this) == false)
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + args[0] + " :You're not channel operator");
        return;
    }
    Client *target = channel->hasClient(args[1]);
    if (!target)
    {
        sendMessage(SERVER_PREFIX, ERR_USERNOTINCHANNEL, _nickname + " " + args[1] + " " + args[0] + " :They aren't on that channel");
        return;
    }

    std::string reason = (args.size() > 2) ? args[2] : args[1];
    channel->sendMessage(getIdentity(), "KICK", args[0] + " " + args[1] + " :" + reason);

    channel->removeClient(target);
    channel->removeOperator(target);
    target->removeChannelRef(channel);
    if (channel->isEmpty())
        Channel::deleteChannel(channel->getChannelName());
}

void Client::invite_command(std::vector<std::string> args)
{
    if (!_is_authenticated)
    {
        sendMessage(SERVER_PREFIX, NOT_REGISTERED, "* :You have not registered");
        return;
    }
    if (args.size() < 2)
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* INVITE :Not enough parameters");
        return;
    }
    Client *target = Client::findClient(args[0]);
    if (!target)
    {
        sendMessage(SERVER_PREFIX, ERR_NOSUCHNICK, _nickname + " " + args[0] + " :No such nick");
        return;
    }
    Channel *channel = Channel::findChannel(args[1]);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, INVALID_NAME, _nickname + " " + args[1] + " :No such channel");
        return;
    }
    if (channel->hasClient(this) == false)
    {
        sendMessage(SERVER_PREFIX, ERR_NOTONCHANNEL, _nickname + " " + args[1] + " :You're not on that channel");
        return;
    }
    if (channel->isOperator(this) == false)
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + args[1] + " :You're not channel operator");
        return;
    }
    if (channel->hasClient(target) == true)
    {
        sendMessage(SERVER_PREFIX, ERR_USERONCHANNEL, _nickname + " " + args[0] + " " + args[1] + " :is already on channel");
        return;
    }

    channel->addInvite(target);
    sendMessage(SERVER_PREFIX, RPL_INVITING, _nickname + " " + args[0] + " " + args[1]);
    target->sendMessage(getIdentity(), "INVITE", args[0] + " " + args[1]);
}

void Client::topic_command(std::vector<std::string> args)
{
    if (!_is_authenticated)
    {
        sendMessage(SERVER_PREFIX, NOT_REGISTERED, "* :You have not registered");
        return;
    }
    if (args.size() < 1)
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* TOPIC :Not enough parameters");
        return;
    }
    Channel *channel = Channel::findChannel(args[0]);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, INVALID_NAME, _nickname + " " + args[0] + " :No such channel");
        return;
    }
    if (channel->hasClient(this) == false)
    {
        sendMessage(SERVER_PREFIX, ERR_NOTONCHANNEL, _nickname + " " + args[0] + " :You're not on that channel");
        return;
    }

    if (args.size() < 2)
    {
        if (channel->getTopic().empty())
            sendMessage(SERVER_PREFIX, RPL_NOTOPIC, _nickname + " " + args[0] + " :No topic is set");
        else
            sendMessage(SERVER_PREFIX, RPL_TOPIC, _nickname + " " + args[0] + " :" + channel->getTopic());
        return;
    }

    if (channel->isTopicRestricted() == true && channel->isOperator(this) == false)
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + args[0] + " :You're not channel operator");
        return;
    }

    channel->setTopic(args[1]);
    channel->sendMessage(getIdentity(), "TOPIC", args[0] + " :" + args[1]);
}
