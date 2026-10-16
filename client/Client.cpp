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
    commands["PING"] = &Client::ping_command;

    return commands;
}

/*
 * Los clientes IRC reales envian PING periodicamente; si no reciben PONG,
 * cierran la conexion por timeout. Respondemos con el mismo token.
 */
void Client::ping_command(std::vector<std::string> args)
{
    if (args.empty())
        sendMessage(SERVER_PREFIX, "PONG", SERVER_PREFIX);
    else
        sendMessage(SERVER_PREFIX, "PONG", std::string(SERVER_PREFIX) + " :" + args[0]);
}

bool Client::handleMenssage(std::string cmd)
{
    std::cout << "Cmd (" << _sock << "): [" << cmd << "]" << std::endl;

    /* ===================== */
    std::stringstream ss;
    std::string main_command;
    ss << cmd;
    ss >> main_command;

    if (main_command.empty())
        return (true);

    /* join / JOIN / JoIn -> misma entrada del mapa */
    main_command = ircToUpper(main_command);

    std::map<std::string, Client::ClientFunction> commands = _get_commands();
    std::map<std::string, ClientFunction>::iterator search = commands.find(main_command);
    if (search != commands.end())
        (this->*(search->second))(parserArgs(ss));
    else
    {
        /* 421: comando desconocido (antes se ignoraba en silencio) */
        std::string nick = _nickname.empty() ? "*" : _nickname;
        sendMessage(SERVER_PREFIX, UNKNOWN_COMMAND, nick + " " + main_command + " :Unknown command");
    }
    /* ===================== */

    return (true);
}

/* ================================================================= */

bool Client::handleClient()
{
    // puede ser POLLIN o POLLHUP
    char buffer[READBUFFER + 1];

    /* Leemos los datos del cliente: mensaje o cierre de conexión */
    ssize_t readBytes = recv(_sock, buffer, READBUFFER, 0);
    if (readBytes <= 0) // leo 0, el cliente se ha ido, si leo < 0 ha habido un error ---> ambos quito la conexion
        return (false);
    buffer[readBytes] = '\0';
    _buffer += buffer; // append: conserva bytes incompletos de recv anteriores

    /*
     * Aceptamos \r\n (IRC) y tambien \n solo (nc sin -C).
     * Si la linea termina en \r (porque llego \r\n), lo quitamos del comando.
     */
    size_t pos;
    while ((pos = _buffer.find('\n')) != std::string::npos)
    {
        std::string cmd = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 1);
        if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
            cmd.erase(cmd.size() - 1);
        if (this->handleMenssage(cmd) == false || _quit == true)
            return (false);
    }
    return (true);
}

bool Client::flushOutgoing()
{
    if (_sendBuffer.empty())
        return (true);
    ssize_t n = send(_sock, _sendBuffer.c_str(), _sendBuffer.size(), 0);
    if (n <= 0)
        return (false);
    _sendBuffer.erase(0, static_cast<size_t>(n));
    return (true);
}

bool Client::hasPendingWrite() const
{
    return (!_sendBuffer.empty());
}

void Client::welcomeMenssage()
{
    _sendBuffer += CLIENT_WELCOME_MENSSAGE;
}

ssize_t Client::sendMessage(std::string prefix, std::string command, std::string arguments)
{
    std::string buffer = ":" + prefix + " " + command + " " + arguments + "\r\n";
    _sendBuffer += buffer;
    return (static_cast<ssize_t>(buffer.size()));
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
