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
#include "Server.hpp"
#include "Client.hpp"

volatile bool g_signal = false;

void signal_handle(int signal)
{
    (void)signal;
    g_signal = true;
}

void removeClient(std::map<int, Client *> &clients, std::vector<pollfd> &fds, std::vector<pollfd>::iterator &itFds)
{
    delete clients.at(itFds->fd);
    clients.erase(clients.find(itFds->fd));
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
    std::map<int, Client *> clients; // solo tiene fds de los clientes
    pollfd server_pollfd = {.fd = server->getSocket(), .events = POLLIN, .revents = 0};
    fds.push_back(server_pollfd);
    while (g_signal == false)
    {
        if (poll(&fds[0], fds.size(), -1) < 0) // error el poll
        {
            if (g_signal == true) // ^C
                break;
            std::cout << "Error in poll" << std::endl;
            for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
                delete it->second;
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
                        // fds.push_back(server.handleNewConection());
                        Client *newClient = server->handleNewConection();
                        clients.insert(std::pair<int, Client *>(newClient->getSocket(), newClient));

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
                        Client *newClient = clients.at(itFds->fd);
                        newClient->welcomeMenssage();
                        itFds->events &= ~POLLOUT;
                    }
                    catch (const std::exception &e)
                    {
                        removeClient(clients, fds, itFds);
                        std::cerr << e.what() << '\n';
                        break;
                    }
                }
                else // cambios el cliente
                {
                    Client *currentClient = clients.at(itFds->fd);
                    if (currentClient->handleClient() == false)
                    {
                        std::cout << "Disconection from client " << itFds->fd << std::endl;
                        removeClient(clients, fds, itFds);
                        break; // como he eliminado fd del vector, tenemos que empezar a leer de nuevo
                    }
                }
            }
        }
    }
    // for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); it++) // cerrar los clientes y el servidor
    //     if(it->fd != server.getSocket())
    //         close(it->fd);
    for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
        delete it->second;
    delete server;
    std::cout << "Server closed" << std::endl;
}
