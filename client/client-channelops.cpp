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

    std::string chanName = args[0];
    std::string targetNick = args[1];
    std::string reason = args.size() > 2 ? args[2] : _nickname;

    Channel *channel = Channel::findChannel(chanName);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, ERR_NOSUCHCHANNEL, _nickname + " " + chanName + " :No such channel");
        return;
    }
    if (!channel->hasClient(this))
    {
        sendMessage(SERVER_PREFIX, ERR_NOTONCHANNEL, _nickname + " " + chanName + " :You're not on that channel");
        return;
    }
    if (!channel->isOperator(this))
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + chanName + " :You're not channel operator");
        return;
    }
    Client *target = channel->hasClient(targetNick);
    if (!target)
    {
        sendMessage(SERVER_PREFIX, ERR_USERNOTINCHANNEL, _nickname + " " + targetNick + " " + chanName + " :They aren't on that channel");
        return;
    }
    if (target == this && channel->getOperatorCount() <= 1)
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + chanName + " :You're the only channel operator, cannot kick yourself");
        return;
    }

    channel->sendMessage(getIdentity(), "KICK", chanName + " " + targetNick + " :" + reason);
    channel->removeClient(target);
    channel->removeOperator(target);
    target->removeChannelRef(channel);
    if (channel->isEmpty())
        Channel::deleteChannel(chanName);
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

    std::string targetNick = args[0];
    std::string chanName = args[1];

    Client *target = Client::findClient(targetNick);
    if (!target)
    {
        sendMessage(SERVER_PREFIX, ERR_NOSUCHNICK, _nickname + " " + targetNick + " :No such nick/channel");
        return;
    }
    Channel *channel = Channel::findChannel(chanName);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, ERR_NOSUCHCHANNEL, _nickname + " " + chanName + " :No such channel");
        return;
    }
    if (!channel->hasClient(this))
    {
        sendMessage(SERVER_PREFIX, ERR_NOTONCHANNEL, _nickname + " " + chanName + " :You're not on that channel");
        return;
    }
    if (channel->hasClient(target))
    {
        sendMessage(SERVER_PREFIX, ERR_USERONCHANNEL, _nickname + " " + targetNick + " " + chanName + " :is already on channel");
        return;
    }
    if (channel->isInviteOnly() && !channel->isOperator(this))
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + chanName + " :You're not channel operator");
        return;
    }

    channel->addInvite(target);
    sendMessage(SERVER_PREFIX, RPL_INVITING, _nickname + " " + targetNick + " " + chanName);
    target->sendMessage(getIdentity(), "INVITE", targetNick + " :" + chanName);
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

    std::string chanName = args[0];
    Channel *channel = Channel::findChannel(chanName);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, ERR_NOSUCHCHANNEL, _nickname + " " + chanName + " :No such channel");
        return;
    }
    if (!channel->hasClient(this))
    {
        sendMessage(SERVER_PREFIX, ERR_NOTONCHANNEL, _nickname + " " + chanName + " :You're not on that channel");
        return;
    }

    if (args.size() < 2)
    {
        if (channel->getTopic().empty())
            sendMessage(SERVER_PREFIX, RPL_NOTOPIC, _nickname + " " + chanName + " :No topic is set");
        else
            sendMessage(SERVER_PREFIX, RPL_TOPIC, _nickname + " " + chanName + " :" + channel->getTopic());
        return;
    }

    if (channel->isTopicRestricted() && !channel->isOperator(this))
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + chanName + " :You're not channel operator");
        return;
    }

    channel->setTopic(args[1]);
    channel->sendMessage(getIdentity(), "TOPIC", chanName + " :" + args[1]);
}
