#include "Client.hpp"
#include "Server.hpp"

std::map<int, Client*> Client::clientsByFd = std::map<int, Client*>();
std::map<std::string, Client*> Client::clientsByNick = std::map<std::string, Client*>();

Client::Client(int sock)
{
    _nickname = "";
    _username = "";
    _hostname = "";
    _servername = "";
    _realname = "";
    _sock = sock;
    _passCorrect = false;
}

Client::~Client()
{
    close(_sock);
    _sock = -1; // como si haga a null con un puntero
}

int Client::getSocket() const
{
    return (_sock);
}

/* ================================================================= */

void Client::pass_command(std::stringstream &args)
{
    std::string tempArg; // argumnets
    std::cout << "Recibido en PASS" << std::endl;
    if(_passCorrect == true) //volver a poner contraseña
    {
        sendMessage(SERVER_PREFIX, ALREADY_REGISTERED, "* :Unauthorized command (already registered)");
        return;
    }
    if (args.rdbuf()->in_avail() <= 0) //vacio
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* PASS :Not enough parameters");
        return;
    }
    args >> tempArg; // first param (password)
    Server *server = Server::getInstance();
    if(tempArg == server->getPass()) // correct
        _passCorrect = true;
    else //incorrect
        sendMessage(SERVER_PREFIX, PASSWORD_DISMATCH, "* :Password incorrect");
}

void Client::nick_command(std::stringstream &args)
{
    std::string temp;
    std::cout << "Recibido en NICK" << std::endl;
    if (args.rdbuf()->in_avail() <= 0) //vacio
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* NICK :Not enough parameters");
        return;
    }
    args >> temp;
    if(Client::findClient(temp) != NULL)
    {
        sendMessage(SERVER_PREFIX, NICKNAME_IN_USE, temp + " " + temp + " :Nickname is already in use");
        return;
    }
    _nickname = temp;
    clientsByNick[_nickname] = this;
}

void Client::user_command(std::stringstream &args)
{
    std::string user;
    std::string host;
    std::string server;
    std::string real = "";
    std::string temp;

    std::cout << "Recibido en USER" << std::endl;
    if(_username.size() > 0)
    {
        sendMessage(SERVER_PREFIX, ALREADY_REGISTERED, "* :Unauthorized command (already registered)");
        return;
    }
    if (args.rdbuf()->in_avail() <= 0) //vacio
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }
    args >> user;
    if(user.empty())
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }

    if (args.rdbuf()->in_avail() <= 0) //vacio
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }
    args >> host;
    if(host.empty())
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }

    if (args.rdbuf()->in_avail() <= 0) //vacio
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }
    args >> server;
    if(server.empty())
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }

    while (args.rdbuf()->in_avail() > 0)
    {
        args >> temp;
        real = real + " " + temp;
    }
    if(real.empty())
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }
    _username = user;
    _hostname = host;
    _servername = server;
    _realname = real;
}

std::map<std::string, Client::ClientFunction> Client::_get_commands()
{
    std::map<std::string, Client::ClientFunction> commands;

    commands["PASS"] = &Client::pass_command;
    commands["NICK"] = &Client::nick_command;
    commands["USER"] = &Client::user_command;

    return commands;
}

/* ================================================================= */

bool Client::handleMenssage(std::string cmd)
{
    std::cout << "Cmd (" << _sock << "): [" << cmd << "]" << std::endl;

    /* ===================== */
    std::stringstream ss;
    std::string main_command;
    ss << cmd;
    ss >> main_command;

    std::map<std::string, Client::ClientFunction> commands = _get_commands();
    std::map<std::string, ClientFunction>::iterator search = commands.find(main_command);
    if (search != commands.end())
        (this->*(search->second))(ss);
    /* ===================== */

    // if (sendMessage("CAP-IRC", "001", ":Hola que tal\n") <= 0)
    //     return (false);
    return (true);
}

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
    for (std::map<int, Client*>::iterator it = clientsByFd.begin(); it != clientsByFd.end(); it++)
        delete it->second;
}

Client *Client::findClient(int sock)
{
    std::map<int, Client *>::iterator it = clientsByFd.find(sock);
    if (it != clientsByFd.end())
        return(it->second);
    return(NULL);
}

Client *Client::findClient(std::string nick)
{
    std::map<std::string, Client *>::iterator it = clientsByNick.find(nick);
    if (it != clientsByNick.end())
        return(it->second);
    return(NULL);
}


bool Client::handleClient()
{
    // puede ser POLLIN o POLLHUP
    char buffer[READBUFFER + 1];
    std::string readed = "";

    /* Leemos los datos del cliente: mensaje o cierre de conexión */
    ssize_t readBytes = recv(_sock, buffer, READBUFFER, 0);
    if (readBytes <= 0) // leo 0, el cliente se ha ido, si leo < 0 ha habido un error ---> ambos quito la conexion
        return (false);
    buffer[readBytes] = '\0';
    _buffer << buffer;

    /* Vemos si tenemos un comando completo */
    bool flag = true;
    do
    {
        flag = false;
        std::string temp = _buffer.str();
        size_t pos = temp.find("\r\n");
        if (pos != std::string::npos)
        {
            std::string cmd = temp.substr(0, pos);
            _buffer.str(temp.substr(pos + 2, temp.size())); // se sobreescribe, no se añade
            if (this->handleMenssage(cmd) == false)
                return (false);
            flag = true;
        }
    } while (flag == true);
    return (true);
}

void Client::welcomeMenssage()
{
    ssize_t nbr = send(_sock, CLIENT_WELCOME_MENSSAGE, strlen(CLIENT_WELCOME_MENSSAGE), 0);
    if (nbr <= 0)
        throw std::runtime_error("Error in send message"); // hacer algo cuando falla
    return;
}

ssize_t Client::sendMessage(std::string prefix, std::string command, std::string arguments)
{
    std::string buffer = ":" + prefix + " " + command + " " + arguments + "\r\n";
    return send(_sock, buffer.c_str(), buffer.size(), 0);
}
