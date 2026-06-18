#include "Client.hpp"
#include "Server.hpp"

std::map<int, Client *> Client::clientsByFd = std::map<int, Client *>();
std::map<std::string, Client *> Client::clientsByNick = std::map<std::string, Client *>();

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

std::vector<std::string> parserArgs(std::stringstream &ss)
{
    std::vector<std::string> args;
    std::string buffer;
    bool flag = false;
    while (ss.rdbuf()->in_avail() > 0)
    {
        ss >> buffer;
        if (flag == false)
        {
            if (buffer[0] == ':')
            {
                flag = true;
                buffer = buffer.substr(1, buffer.size() - 1);
            }
            args.push_back(buffer);
        }
        else
            args[args.size() - 1] += " " + buffer;
    }
    return (args);
}

void Client::pass_command(std::vector<std::string> args)
{
    std::cout << "Recibido en PASS" << std::endl;
    if (_passCorrect == true) // volver a poner contraseña
    {
        sendMessage(SERVER_PREFIX, ALREADY_REGISTERED, "* :Unauthorized command (already registered)");
        return;
    }
    if (args.size() < 1) // vacio
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* PASS :Not enough parameters");
        return;
    }
    Server *server = Server::getInstance();
    if (args[0] == server->getPass()) // correct
        _passCorrect = true;
    else // incorrect
        sendMessage(SERVER_PREFIX, PASSWORD_DISMATCH, "* :Password incorrect");
}

void Client::removeNickname(Client *c)
{
    if (c->_nickname == "")
        return;
    std::map<std::string, Client *>::iterator it = clientsByNick.find(c->_nickname);
    clientsByNick.erase(it);
}

void Client::nick_command(std::vector<std::string> args)
{
    std::cout << "Recibido en NICK" << std::endl;
    if (args.size() < 1) // vacio
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* NICK :Not enough parameters");
        return;
    }
    if (Client::findClient(args[0]) != NULL) // repeat client
    {
        sendMessage(SERVER_PREFIX, NICKNAME_IN_USE, args[0] + " " + args[0] + " :Nickname is already in use");
        return;
    }
    removeNickname(this);
    _nickname = args[0];
    clientsByNick[_nickname] = this;
}

void Client::user_command(std::vector<std::string> args)
{
    std::cout << "Recibido en USER" << std::endl;
    if (_username.size() > 0)
    {
        sendMessage(SERVER_PREFIX, ALREADY_REGISTERED, "* :Unauthorized command (already registered)");
        return;
    }
    if (args.size() < 4)
    {
        sendMessage(SERVER_PREFIX, NEED_MORE_PARAMS, "* USER :Not enough parameters");
        return;
    }

    _username = args[0];
    _hostname = args[1];
    _servername = args[2];
    _realname = args[3];
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
        (this->*(search->second))(parserArgs(ss));
    /* ===================== */

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
