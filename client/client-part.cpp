#include "Client.hpp"

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

    std::string arguments = args[0];
    if (args.size() > 1)
        arguments += " :" + args[1];
    channel->sendMessage(getIdentity(), "PART", arguments);

    channel->removeClient(this);
    channel->removeOperator(this);
    removeChannelRef(channel);
    if (channel->isEmpty())
        Channel::deleteChannel(channel->getChannelName());
}

void Client::quit_command(std::vector<std::string> args)
{
    _quitReason = (args.size() > 0) ? args[0] : "Client Quit";
    _quit = true;
}
