#include "Client.hpp"

Client::Client(int sock)
{
    _nickname = "";
    _username = "";
    _hostname = "";
    _servername = "";
    _realname = "";
    _sock = sock;
    _passCorrect = false;
    _is_authenticated = false;
    _quit = false;
    _quitReason = "Leaving";
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

std::string Client::getNickname() const
{
    return (_nickname);
}

std::string Client::getIdentity() const
{
    return(_nickname + "!" + _username + "@" + _hostname);
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

std::map<std::string, Client::ClientFunction> Client::_get_commands()
{
    std::map<std::string, Client::ClientFunction> commands;

    commands["PASS"] = &Client::pass_command;
    commands["NICK"] = &Client::nick_command;
    commands["USER"] = &Client::user_command;
    commands["JOIN"] = &Client::join_command;
    commands["PRIVMSG"] = &Client::privmsg_command;
    commands["PART"] = &Client::part_command;
    commands["QUIT"] = &Client::quit_command;
    commands["KICK"] = &Client::kick_command;
    commands["INVITE"] = &Client::invite_command;
    commands["TOPIC"] = &Client::topic_command;
    commands["MODE"] = &Client::mode_command;

    return commands;
}

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

/* ================================================================= */

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
            if (this->handleMenssage(cmd) == false || _quit == true)
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

/* ================================================================= */

void Client::addChannelRef(Channel *channel)
{
    _channels.push_back(channel);
}

void Client::removeChannelRef(Channel *channel)
{
    for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (*it == channel)
        {
            _channels.erase(it);
            return;
        }
    }
}

void Client::leaveAllChannels(std::string reason)
{
    for (size_t i = 0; i < _channels.size(); i++)
    {
        Channel *channel = _channels[i];
        channel->sendMessage(getIdentity(), "QUIT", ":" + reason);
        channel->removeClient(this);
        channel->removeOperator(this);
        if (channel->isEmpty())
            Channel::deleteChannel(channel->getChannelName());
    }
    _channels.clear();
}
