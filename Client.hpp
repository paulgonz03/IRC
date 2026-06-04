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


#define CLIENT_WELCOME_MENSSAGE "Welcome to my IRC, enter your login and pasword:\n"
#define READBUFFER 1024

class Client
{
private:
    int _sock;
    std::stringstream _buffer;
    // std::string _nickname;
    // std::string _username;
    // std::string _hostname;
    // std::string _servername;
    // std::string _realname;

public:
    Client(int sock);
    ~Client();

    int getSocket(void) const;
    bool handleClient();
    bool handleMenssage(std::string cmd);
    void welcomeMenssage();

    ssize_t sendMessage( std::string prefix, std::string command, std::string arguments );
};


