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

    std::string chanName = args[0];
    Channel *channel = Channel::findChannel(chanName);
    if (!channel)
    {
        sendMessage(SERVER_PREFIX, ERR_NOSUCHCHANNEL, _nickname + " " + chanName + " :No such channel");
        return;
    }

    if (args.size() < 2) // consultar modos actuales
    {
        sendMessage(SERVER_PREFIX, RPL_CHANNELMODEIS, _nickname + " " + chanName + " " + channel->getModeString());
        return;
    }

    if (!channel->isOperator(this))
    {
        sendMessage(SERVER_PREFIX, ERR_CHANOPRIVSNEEDED, _nickname + " " + chanName + " :You're not channel operator");
        return;
    }

    std::string modeStr = args[1];
    size_t paramIdx = 2;
    bool adding = true;

    std::vector<bool> signs;
    std::vector<char> modeChars;
    std::vector<std::string> params;

    for (size_t i = 0; i < modeStr.size(); i++)
    {
        char c = modeStr[i];

        if (c == '+')
        {
            adding = true;
            continue;
        }
        if (c == '-')
        {
            adding = false;
            continue;
        }

        if (c == 'i')
        {
            channel->setInviteOnly(adding);
            signs.push_back(adding);
            modeChars.push_back(c);
            params.push_back("");
        }
        else if (c == 't')
        {
            channel->setTopicRestricted(adding);
            signs.push_back(adding);
            modeChars.push_back(c);
            params.push_back("");
        }
        else if (c == 'k')
        {
            if (adding)
            {
                if (paramIdx >= args.size())
                {
                    sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, _nickname + " MODE :Not enough parameters");
                    continue;
                }
                channel->setPassword(args[paramIdx]);
                signs.push_back(true);
                modeChars.push_back(c);
                params.push_back(args[paramIdx]);
                paramIdx++;
            }
            else
            {
                channel->unsetPassword();
                signs.push_back(false);
                modeChars.push_back(c);
                params.push_back("");
            }
        }
        else if (c == 'l')
        {
            if (adding)
            {
                if (paramIdx >= args.size())
                {
                    sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, _nickname + " MODE :Not enough parameters");
                    continue;
                }
                long limit = std::atol(args[paramIdx].c_str());
                if (limit < 0)
                    limit = 0;
                channel->setLimit((size_t)limit);
                signs.push_back(true);
                modeChars.push_back(c);
                params.push_back(args[paramIdx]);
                paramIdx++;
            }
            else
            {
                channel->unsetLimit();
                signs.push_back(false);
                modeChars.push_back(c);
                params.push_back("");
            }
        }
        else if (c == 'o')
        {
            if (paramIdx >= args.size())
            {
                sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, _nickname + " MODE :Not enough parameters");
                continue;
            }
            std::string targetNick = args[paramIdx];
            paramIdx++;
            Client *target = channel->hasClient(targetNick);
            if (!target)
            {
                sendMessage(SERVER_PREFIX, ERR_USERNOTINCHANNEL, _nickname + " " + targetNick + " " + chanName + " :They aren't on that channel");
                continue;
            }
            if (adding)
                channel->addOperator(target);
            else
                channel->removeOperator(target);
            signs.push_back(adding);
            modeChars.push_back(c);
            params.push_back(targetNick);
        }
        else
        {
            sendMessage(SERVER_PREFIX, ERR_UNKNOWNMODE, _nickname + " " + std::string(1, c) + " :is unknown mode char to me");
        }
    }

    if (modeChars.empty())
        return;

    std::string out = "";
    std::string outParams = "";
    bool curSign = signs[0];
    bool first = true;

    for (size_t i = 0; i < modeChars.size(); i++)
    {
        if (first || signs[i] != curSign)
        {
            out += signs[i] ? '+' : '-';
            curSign = signs[i];
            first = false;
        }
        out += modeChars[i];
        if (!params[i].empty())
            outParams += " " + params[i];
    }

    channel->sendMessage(getIdentity(), "MODE", chanName + " " + out + outParams);
}
