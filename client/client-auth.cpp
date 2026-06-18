#include "Client.hpp"

void Client::checkAuth()
{
    if(_nickname.empty() || _username.empty() || _hostname.empty() || _servername.empty() || _realname.empty() || _passCorrect == false || _sendmessage == true)
        return ;
    
    sendMessage(SERVER_PREFIX, REPLY_WELCOME, _nickname + " :Welcome to the Internet Relay Network " +  _nickname + "!" + _username + "@" + _hostname);
    sendMessage(SERVER_PREFIX, RPL_YOURHOST,  _nickname + " :Your host is " + SERVER_PREFIX + ", running version " + SERVER_VERSION);
    // sendMessage(SERVER_PREFIX, RPL_CREATED, "Welcome to the Internet Relay Network paulgonz!paulgonz@localhost");
    sendMessage(SERVER_PREFIX, RPL_MYINFO, _nickname + " " + SERVER_PREFIX + " " + SERVER_VERSION + " i tkoitl");
    _sendmessage = true;
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
