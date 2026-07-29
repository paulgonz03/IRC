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
    if (args.size() < 2 || args[1].empty())
    {
        sendMessage(SERVER_PREFIX, ERR_NOTEXTTOSEND, _nickname + " :No text to send");
        return;
    }

    std::string text = args[1];
    std::stringstream ss(args[0]);
    std::string target;
    while (std::getline(ss, target, ','))
    {
        if (target.empty())
            continue;
        if (target[0] == '#')
        {
            Channel *channel = Channel::findChannel(target);
            if (!channel)
            {
                sendMessage(SERVER_PREFIX, ERR_NOSUCHCHANNEL, _nickname + " " + target + " :No such channel");
                continue;
            }
            if (!channel->hasClient(this))
            {
                sendMessage(SERVER_PREFIX, ERR_CANNOTSENDTOCHAN, _nickname + " " + target + " :Cannot send to channel");
                continue;
            }
            channel->sendMessageExcept(this, getIdentity(), "PRIVMSG", target + " :" + text);
        }
        else
        {
            Client *dest = Client::findClient(target);
            if (!dest)
            {
                sendMessage(SERVER_PREFIX, ERR_NOSUCHNICK, _nickname + " " + target + " :No such nick/channel");
                continue;
            }
            dest->sendMessage(getIdentity(), "PRIVMSG", target + " :" + text);
        }
    }
}

void Client::ping_command(std::vector<std::string> args)
{
    std::string token = args.size() > 0 ? args[0] : SERVER_PREFIX;
    sendRaw("PONG " + std::string(SERVER_PREFIX) + " :" + token);
}
