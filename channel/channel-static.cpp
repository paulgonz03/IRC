#include "Channel.hpp"

std::map<std::string, Channel *> Channel::_channels = std::map<std::string, Channel *>();

Channel *Channel::addChannel(std::string name)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(name);
    if(it == _channels.end())
    {
        Channel *channel = new Channel(name);
        _channels[name] = channel;
        return (channel);
    }
    return(NULL);
}

void Channel::deleteChannel(std::string name)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(name);
    if(it == _channels.end())
        return ;
    delete it->second;
    _channels.erase(it);
}

void Channel::deleteChannels()
{
    for(std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
        delete it->second;
    _channels.clear();
}

Channel *Channel::findChannel(std::string name)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(name);
    if(it == _channels.end())
        return NULL;
    return(it->second);
}
