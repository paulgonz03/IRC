#include "Client.hpp"

std::map<int, Client *> Client::clientsByFd = std::map<int, Client *>();
std::map<std::string, Client *> Client::clientsByNick = std::map<std::string, Client *>();

Client *Client::addClient(int sock)
{
    Client *newClient = new Client(sock);
    clientsByFd[sock] = newClient;
    return newClient;
}

void Client::deleteClient(int sock)
{
    std::map<int, Client *>::iterator itFd = clientsByFd.find(sock);
    std::map<std::string, Client *>::iterator itNick = clientsByNick.find(itFd->second->_nickname);
    if (itNick != clientsByNick.end())
        clientsByNick.erase(itNick);
    delete itFd->second;
    clientsByFd.erase(itFd);
}

void Client::deleteClients()
{
    for (std::map<int, Client *>::iterator it = clientsByFd.begin(); it != clientsByFd.end(); it++)
        delete it->second;
}

Client *Client::findClient(int sock)
{
    std::map<int, Client *>::iterator it = clientsByFd.find(sock);
    if (it != clientsByFd.end())
        return (it->second);
    return (NULL);
}

Client *Client::findClient(std::string nick)
{
    std::map<std::string, Client *>::iterator it = clientsByNick.find(nick);
    if (it != clientsByNick.end())
        return (it->second);
    return (NULL);
}

void Client::removeNickname(Client *c)
{
    if (c->_nickname == "")
        return;
    std::map<std::string, Client *>::iterator it = clientsByNick.find(c->_nickname);
    clientsByNick.erase(it);
}
