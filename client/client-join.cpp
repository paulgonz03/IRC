#include "Client.hpp"


void Client::sendJoinMessage(Channel *channel)
{
    channel->sendMessage(getIdentity(), "JOIN", channel->getChannelName());
    sendMessage(SERVER_PREFIX, RPL_NOTOPIC, _nickname + " " + channel->getChannelName() + " :No topic is set");
    sendMessage(SERVER_PREFIX, RPL_NAMREPLY, _nickname + " = " + channel->getChannelName() + " :" + channel->getAllClients());
    sendMessage(SERVER_PREFIX, RPL_ENDOFNAMES, _nickname + " " + channel->getChannelName() + " :End of /NAMES list");
}

void Client::join_command(std::vector<std::string> args)
{
    if (!_is_authenticated)
    {
        sendMessage(SERVER_PREFIX, NOT_REGISTERED, "* :You have not registered");
        return;
    }

    if (args.size() < 1)
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* JOIN :Not enough parameters");
        return;
    }

    Channel *channel = Channel::findChannel(args[0]);
    if (!channel) // no existe el canal
    {
        channel = Channel::addChannel(args[0]);
        channel->addClient(this);
        channel->addOperator(this);
        sendJoinMessage(channel);
        return;
    }
    if (channel->hasClient(this) == true) //existe el canal pero estoy dentro de el ya
        return;

}
