#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <signal.h>
#include <cctype>
#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <map>
#include "../irc_codes.hpp"
#include "../server/Server.hpp"
#include "../channel/Channel.hpp"

#define CLIENT_WELCOME_MENSSAGE "Welcome to my IRC, enter your login and pasword:\r\n"
#define READBUFFER 1024

class Server;
class Channel;

/* IRC trata comandos y nicks sin distinguir mayusculas/minusculas */
inline std::string ircToLower(const std::string &s)
{
    std::string out(s);
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return out;
}

inline std::string ircToUpper(const std::string &s)
{
    std::string out(s);
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[i])));
    return out;
}

class Client
{
public:
    typedef void (Client::*ClientFunction)(std::vector<std::string>);

private:
    int _sock;
    bool _passCorrect;
    std::string _buffer;
    std::string _sendBuffer; /* cola de escritura; se vacia solo con POLLOUT */

    std::string _nickname;
    std::string _username;
    std::string _hostname;
    std::string _servername;
    std::string _realname;

    bool _is_authenticated;
    bool _quit;
    std::string _quitReason;

    std::vector<Channel *> _channels;

    static std::map<int, Client *> clientsByFd;
    static std::map<std::string, Client *> clientsByNick; /* clave en minusculas */
    std::map<std::string, ClientFunction> _get_commands();

public:
    Client(int sock);
    ~Client();

    int getSocket() const;
    std::string getNickname() const;
    std::string getIdentity() const;
    bool handleClient();
    bool handleMenssage(std::string cmd);
    void welcomeMenssage();
    ssize_t sendMessage(std::string prefix, std::string command, std::string arguments);
    bool flushOutgoing();
    bool hasPendingWrite() const;

    void sendJoinMessage(Channel *channel);

    /* Channel membership tracking */
    void addChannelRef(Channel *channel);
    void removeChannelRef(Channel *channel);
    void leaveAllChannels(std::string reason);

    void checkAuth();
    void pass_command(std::vector<std::string> args);
    void nick_command(std::vector<std::string> args);
    void user_command(std::vector<std::string> args);
    void join_command(std::vector<std::string> args);
    void privmsg_command(std::vector<std::string> args);
    void part_command(std::vector<std::string> args);
    void quit_command(std::vector<std::string> args);
    void kick_command(std::vector<std::string> args);
    void invite_command(std::vector<std::string> args);
    void topic_command(std::vector<std::string> args);
    void mode_command(std::vector<std::string> args);
    void ping_command(std::vector<std::string> args);


    static Client *addClient(int sock);
    static void deleteClient(int sock);
    static void deleteClients();
    static Client *findClient(int sock);
    static Client *findClient(std::string nick);
    static void removeNickname(Client *c);


};
