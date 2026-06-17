#include "Client.hpp"

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
    std::string temp;
    args >> temp;
    std::cout << "Recibido en PASS: [" << temp << "]" << std::endl;

    // if (args.rdbuf()->in_avail() <= 0)
        // ERROR -> Enviar mensaje de error de falta de parametros
}

void Client::nick_command(std::stringstream &args)
{
    std::string temp;
    args >> temp;
    std::cout << "Recibido en NICK: [" << temp << "]" << std::endl;
}

void Client::user_command(std::stringstream &args)
{
    std::string temp;
    args >> temp;
    std::cout << "Recibido en USER: [" << temp << "]" << std::endl;
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
    std::string buffer = ":" + prefix + " " + command + " " + arguments;
    return send(_sock, buffer.c_str(), buffer.size(), 0);
}
