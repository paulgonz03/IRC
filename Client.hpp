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
#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <map>
#include "irc_codes.hpp"

#define CLIENT_WELCOME_MENSSAGE "Welcome to my IRC, enter your login and pasword:\n"
#define READBUFFER 1024

class Client
{
public:
    typedef void (Client::*ClientFunction)(std::vector<std::string>);

private:
    int _sock;
    bool _passCorrect;
    std::stringstream _buffer;
    std::string _nickname;
    std::string _username;
    std::string _hostname;
    std::string _servername;
    std::string _realname;

    static std::map<int, Client *> clientsByFd;
    static std::map<std::string, Client *> clientsByNick;
    std::map<std::string, ClientFunction> _get_commands();

public:
    Client(int sock);
    ~Client();

    int getSocket(void) const;
    bool handleClient();
    bool handleMenssage(std::string cmd);
    void welcomeMenssage();

    ssize_t sendMessage(std::string prefix, std::string command, std::string arguments);
    void pass_command(std::vector<std::string> args);
    void nick_command(std::vector<std::string> args);
    void user_command(std::vector<std::string> args);

    static Client *addClient(int sock);
    static void deleteClient(int sock);
    static void deleteClients();
    static Client *findClient(int sock);
    static Client *findClient(std::string nick);

    void removeNickname(Client *c);
};
