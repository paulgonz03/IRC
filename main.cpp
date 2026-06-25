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
#include <map>
#include "server/Server.hpp"
#include "client/Client.hpp"
#include "channel/Channel.hpp"

volatile bool g_signal = false;

void signal_handle(int signal)
{
    (void)signal;
    g_signal = true;
}

void removeClient(std::vector<pollfd> &fds, std::vector<pollfd>::iterator &itFds)
{
    Client::deleteClient(itFds->fd);
    fds.erase(itFds);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return (1);
    }
    signal(SIGINT, signal_handle);
    Server *server = Server::getInstance();
    try
    {
        server->run(argv[1], argv[2]);
    }
    catch (const std::exception &e)
    {
        delete server;
        std::cerr << e.what() << '\n';
        return (1);
    }
    std::cout << "Server started" << std::endl;
    /*
    escucha (poll)
    */
    std::vector<pollfd> fds;         // tiene todos los fds
    // std::map<int, Client *> clients; // solo tiene fds de los clientes
    pollfd server_pollfd = {.fd = server->getSocket(), .events = POLLIN, .revents = 0};
    fds.push_back(server_pollfd);
    while (g_signal == false)
    {
        if (poll(&fds[0], fds.size(), -1) < 0) // error el poll
        {
            if (g_signal == true) // ^C
                break;
            std::cout << "Error in poll" << std::endl;
            Channel::deleteChannels();
            Client::deleteClients();
            delete server;
            return (1);
        }
        for (std::vector<pollfd>::iterator itFds = fds.begin(); itFds != fds.end(); itFds++)
        {
            if ((itFds->revents & POLLIN) || (itFds->revents & POLLHUP) || (itFds->revents & POLLOUT))
            {
                if (itFds->fd == server->getSocket()) // hay nueva conexion
                {
                    try
                    {
                        Client *newClient = server->handleNewConection();

                        pollfd newClientPollfd = {.fd = newClient->getSocket(), .events = POLLIN | POLLHUP | POLLOUT, .revents = 0};
                        fds.push_back(newClientPollfd);
                        break; // quiero que el iterador empieze de nuevo por si se han cambiado las direcciones de memoria
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
                else if (itFds->revents & POLLOUT) // nueva conexion, mandar mensaje de bienvenida
                {
                    try
                    {
                        Client *newClient = Client::findClient(itFds->fd);
                        newClient->welcomeMenssage();
                        itFds->events &= ~POLLOUT;
                    }
                    catch (const std::exception &e)
                    {
                        removeClient(fds, itFds);
                        std::cerr << e.what() << '\n';
                        break;
                    }
                }
                else // cambios el cliente
                {
                    Client *currentClient = Client::findClient(itFds->fd);
                    if (currentClient->handleClient() == false)
                    {
                        std::cout << "Disconection from client " << itFds->fd << std::endl;
                        removeClient(fds, itFds);
                        break; // como he eliminado fd del vector, tenemos que empezar a leer de nuevo
                    }
                }
            }
        }
    }
    Channel::deleteChannels();
    Client::deleteClients();
    delete server;
    std::cout << "Server closed" << std::endl;
}
