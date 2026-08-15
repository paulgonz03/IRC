#include "Client.hpp"

void Client::mode_command(std::vector<std::string> args)
{
    if (!_is_authenticated)
    {
        sendMessage(SERVER_PREFIX, NOT_REGISTERED, "* :You have not registered");
        return;
    }
    if (args.size() < 1)
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* MODE :Not enough parameters");
        return;
    }
    Channel *channel = Channel::findChannel(args[0]);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, INVALID_NAME, _nickname + " " + args[0] + " :No such channel");
        return;
    }

    if (args.size() < 2)
    {
        sendMessage(SERVER_PREFIX, RPL_CHANNELMODEIS, _nickname + " " + args[0] + " " + channel->getModeString());
        return;
    }
    if (channel->isOperator(this) == false)
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + args[0] + " :You're not channel operator");
        return;
    }

    std::string modes = args[1];
    size_t paramIndex = 2;
    bool adding = true;
    std::string appliedPlus = "";
    std::string appliedMinus = "";
    std::string appliedParams = "";

    for (size_t i = 0; i < modes.size(); i++)
    {
        char m = modes[i];
        if (m == '+')
        {
            adding = true;
            continue;
        }
        if (m == '-')
        {
            adding = false;
            continue;
        }
        if (m == 'i')
        {
            if (adding)
                channel->setMode(MODE_I);
            else
                channel->unsetMode(MODE_I);
            (adding ? appliedPlus : appliedMinus) += "i";
        }
        else if (m == 't')
        {
            channel->setTopicRestricted(adding);
            (adding ? appliedPlus : appliedMinus) += "t";
        }
        else if (m == 'k')
        {
            if (adding)
            {
                if (paramIndex >= args.size())
                    continue;
                channel->setKey(args[paramIndex]);
                channel->setMode(MODE_K);
                appliedPlus += "k";
                appliedParams += " " + args[paramIndex];
                paramIndex++;
            }
            else
            {
                channel->unsetMode(MODE_K);
                channel->setKey("");
                appliedMinus += "k";
            }
        }
        else if (m == 'l')
        {
            if (adding)
            {
                if (paramIndex >= args.size())
                    continue;
                channel->setLimit(static_cast<size_t>(atoi(args[paramIndex].c_str())));
                channel->setMode(MODE_L);
                appliedPlus += "l";
                appliedParams += " " + args[paramIndex];
                paramIndex++;
            }
            else
            {
                channel->unsetMode(MODE_L);
                appliedMinus += "l";
            }
        }
        else if (m == 'o')
        {
            if (paramIndex >= args.size())
                continue;
            Client *target = channel->hasClient(args[paramIndex]);
            if (target)
            {
                if (adding)
                    channel->addOperator(target);
                else
                    channel->removeOperator(target);
                (adding ? appliedPlus : appliedMinus) += "o";
                appliedParams += " " + args[paramIndex];
            }
            paramIndex++;
        }
        else
        {
            sendMessage(SERVER_PREFIX, ERR_UNKNOWNMODE, _nickname + " " + std::string(1, m) + " :is unknown mode char to me");
        }
    }

    std::string result = "";
    if (!appliedPlus.empty())
        result += "+" + appliedPlus;
    if (!appliedMinus.empty())
        result += "-" + appliedMinus;
    if (!result.empty())
        channel->sendMessage(getIdentity(), "MODE", args[0] + " " + result + appliedParams);
}
