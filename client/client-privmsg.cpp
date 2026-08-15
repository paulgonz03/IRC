#include "Client.hpp"

void Client::privmsg_command(std::vector<std::string> args)
{
    if (!_is_authenticated)
    {
        sendMessage(SERVER_PREFIX, NOT_REGISTERED, "* :You have not registered");
        return;
    }
    if (args.size() < 1)
    {
        sendMessage(SERVER_PREFIX, ERR_NORECIPIENT, _nickname + " :No recipient given (PRIVMSG)");
        return;
    }
    if (args.size() < 2)
    {
        sendMessage(SERVER_PREFIX, ERR_NOTEXTTOSEND, _nickname + " :No text to send");
        return;
    }

    std::string target = args[0];
    std::string text = args[1];

    if (target[0] == '#')
    {
        Channel *channel = Channel::findChannel(target);
        if (!channel)
        {
            sendMessage(SERVER_PREFIX, INVALID_NAME, _nickname + " " + target + " :No such channel");
            return;
        }
        if (channel->hasClient(this) == false)
        {
            sendMessage(SERVER_PREFIX, ERR_CANNOTSENDTOCHAN, _nickname + " " + target + " :Cannot send to channel");
            return;
        }
        channel->sendMessageExcept(this, getIdentity(), "PRIVMSG", target + " :" + text);
        return;
    }

    Client *dest = Client::findClient(target);
    if (!dest)
    {
        sendMessage(SERVER_PREFIX, ERR_NOSUCHNICK, _nickname + " " + target + " :No such nick");
        return;
    }
    dest->sendMessage(getIdentity(), "PRIVMSG", target + " :" + text);
}
