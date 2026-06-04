#include "Server.hpp"

std::string const Server::IP = "0.0.0.0";
Server* Server::_instance = NULL;

Server::Server()
{}

Server::~Server()
{
    close(_sock);
    _sock = -1; // como si haga a null con un puntero
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

    /*
    le hacemos no bloqueante(que no bloque procesos si hay algun error)
    */
    // 2 parametro - que quiero hacer en el fd
    // 3 parametro - parametro que determina si quiero cambiar la configuracion del fd, por ejemplo un set
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
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
        throw std::out_of_range("Invalid port");

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
    // hay que hacer que el cliente sea no bloqueante
    int flags = fcntl(clientSock, F_GETFL, 0);
    if (flags < 0 || fcntl(clientSock, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("Error making nonblocking socket");

    // pollfd newClient = {.fd = clientSock, .events = POLLIN | POLLHUP, .revents = 0};
    return new Client(clientSock);
    // return (newClient);
}

Server* Server::getInstance()
{
    if (_instance)
        return(_instance);
    _instance = new Server();
    return(_instance);
}