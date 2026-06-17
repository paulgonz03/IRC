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
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include "Client.hpp"

class Server
{
private:
    int _sock;
    std::string _password;
    
    static Server* _instance;
    Server();

public:
    static const std::string IP;

    ~Server();

    void run(std::string port, std::string password);
    int getSocket(void) const;
    std::string getPass(void) const;
    Client* handleNewConection();

    static Server* getInstance();
};
