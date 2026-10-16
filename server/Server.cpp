#include "Server.hpp"

std::string const Server::IP = "0.0.0.0";
Server* Server::_instance = NULL;

Server::Server() : _sock(-1)
{}

Server::~Server()
{
    if (_sock >= 0)
        close(_sock);
    _sock = -1;
}

int Server::getSocket() const
{
    return(_sock);
}

void Server::run(std::string port, std::string password)
{
    _password = password;

    /*
    crear el socket
    */
    // 1 parametro especificar la interfac 127.0.0.1 (IPv4)
    // 2 parametro decir que quiero que haya info en ambos sentidos (como es la comunicacion)
    // 3 parametro protocolo, es un 0

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("Error creating the socket");

    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(fd);
        throw std::runtime_error("Error making nonblocking socket");
    }

    /*
    configurarlo (hacerlo reusable --> hacer que no haya timeout)
    */
    int temp = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(temp)) < 0)
    {
        close(fd);
        throw std::runtime_error("Error in setsockopt");
    }

    /*
    especificar puerto e IP
    65535 cantida de puertos que puede tener un sistema
    */

    int intPort = std::atoi(port.c_str());
    if (intPort < 1 || intPort > 65535)
    {
        close(fd);
        throw std::out_of_range("Invalid port");
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(intPort);
    if (inet_pton(AF_INET, Server::IP.c_str(), &addr.sin_addr) < 0)
    {
        close(fd);
        throw std::runtime_error("IP error");
    }

    /*
    enlazarlo y activarlo
    */
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(fd);
        throw std::runtime_error("Error in binding socket");
    }
    // SOMAXCONN: numero de usuarios que se pueden conectar como maximo
    if (listen(fd, SOMAXCONN) < 0)
    {
        close(fd);
        throw std::runtime_error("Error in listening socket");
    }
    _sock = fd;
}

Client* Server::handleNewConection()
{
    int clientSock = accept(_sock, NULL, NULL);
    std::cout << "New client: " << clientSock << std::endl;
    if (clientSock < 0)
        throw std::runtime_error("Error accepting new client");
    if (fcntl(clientSock, F_SETFL, O_NONBLOCK) < 0)
    {
        close(clientSock);
        throw std::runtime_error("Error making nonblocking socket");
    }
    return (Client::addClient(clientSock));
}

Server* Server::getInstance()
{
    if (_instance)
        return(_instance);
    _instance = new Server();
    return(_instance);
}

std::string Server::getPass(void) const
{
    return(_password);
}