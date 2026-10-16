#include "Client.hpp"

void Client::checkAuth()
{
    if(_nickname.empty() || _username.empty() || _hostname.empty() || _servername.empty() || _realname.empty() || _passCorrect == false || _is_authenticated == true)
        return ;
    sendMessage(SERVER_PREFIX, REPLY_WELCOME, _nickname + " :Welcome to the Internet Relay Network " +  _nickname + "!" + _username + "@" + _hostname);
    sendMessage(SERVER_PREFIX, RPL_YOURHOST,  _nickname + " :Your host is " + SERVER_PREFIX + ", running version " + SERVER_VERSION);
    // sendMessage(SERVER_PREFIX, RPL_CREATED, "Welcome to the Internet Relay Network paulgonz!paulgonz@localhost");
    sendMessage(SERVER_PREFIX, RPL_MYINFO, _nickname + " " + SERVER_PREFIX + " " + SERVER_VERSION + " o o itkol");
    _is_authenticated = true;
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
    {
        _passCorrect = true;
        checkAuth();
    }
    else // incorrect
        sendMessage(SERVER_PREFIX, PASSWORD_DISMATCH, "* :Password incorrect");
}

int CheckNick(std::string arg)
{
    if (arg.empty() || arg.size() > 9)
        return(0);
    for (size_t i = 0; i < arg.size(); i++) 
        if(arg[i] == ' ' || arg[i] == ',' || arg[i] == ':')
            return (0);
    if(arg[0] == '#' || arg[0] == '&' || arg[0] == ':' || std::isdigit(static_cast<unsigned char>(arg[0])))
        return(0);
    return(1);
}

void Client::nick_command(std::vector<std::string> args)
{
    std::cout << "Recibido en NICK" << std::endl;
    if (args.size() < 1) // vacio
    {
        sendMessage(SERVER_PREFIX, ERR_NONICKNAMEGIVEN, "* :No nickname given");
        return;
    }
    if(!CheckNick(args[0]))
    {
        sendMessage(SERVER_PREFIX, ERR_ERRONEUSNICKNAME, std::string("* ") + args[0] + " :Erroneous nickname");
        return;
    }
    /* Bob y bob son el mismo nick; permitir renombrarse a si mismo (cambio de case) */
    Client *existing = Client::findClient(args[0]);
    if (existing != NULL && existing != this)
    {
        sendMessage(SERVER_PREFIX, NICKNAME_IN_USE, args[0] + " " + args[0] + " :Nickname is already in use");
        return;
    }

    std::string oldNick = _nickname;
    std::string oldIdentity = getIdentity(); // identidad ANTES del cambio (para el broadcast)

    removeNickname(this);
    _nickname = args[0];
    clientsByNick[ircToLower(_nickname)] = this;

    /* Si ya tenia nick y esta en canales: actualizar mapas y avisar a los demas */
    if (!oldNick.empty())
    {
        for (size_t i = 0; i < _channels.size(); i++)
            _channels[i]->updateClientNickname(this, oldNick);
        /* Aviso al propio cliente una vez */
        sendMessage(oldIdentity, "NICK", _nickname);
        /* Aviso al resto de cada canal (excepto yo, para no duplicar) */
        for (size_t i = 0; i < _channels.size(); i++)
            _channels[i]->sendMessageExcept(this, oldIdentity, "NICK", _nickname);
    }

    checkAuth();
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
    checkAuth();
}
