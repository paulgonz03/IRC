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

/* Activa o quita POLLOUT segun quede algo en la cola de envio del cliente */
void updatePollOut(pollfd &pfd, Client *client)
{
    if (client->hasPendingWrite())
        pfd.events |= POLLOUT;
    else
        pfd.events &= ~POLLOUT;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return (1);
    }
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = signal_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* sin SA_RESTART: poll sale y se puede cerrar con Ctrl+C */
    sigaction(SIGINT, &sa, NULL);

    struct sigaction ign;
    bzero(&ign, sizeof(ign));
    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    sigaction(SIGPIPE, &ign, NULL);
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
    pollfd server_pollfd;
    server_pollfd.fd = server->getSocket();
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;
    fds.push_back(server_pollfd);
    while (g_signal == false)
    {
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
        {
            if (it->fd == server->getSocket())
                continue;
            Client *queued = Client::findClient(it->fd);
            if (queued != NULL)
                updatePollOut(*it, queued);
        }
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
            if (itFds->revents & (POLLIN | POLLHUP | POLLOUT | POLLERR | POLLNVAL))
            {
                if (itFds->fd == server->getSocket()) // hay nueva conexion
                {
                    try
                    {
                        Client *newClient = server->handleNewConection();
                        newClient->welcomeMenssage(); /* se envia cuando poll marque POLLOUT */

                        pollfd newClientPollfd;
                        newClientPollfd.fd = newClient->getSocket();
                        newClientPollfd.events = POLLIN | POLLHUP;
                        newClientPollfd.revents = 0;
                        updatePollOut(newClientPollfd, newClient);
                        fds.push_back(newClientPollfd);
                        break; // quiero que el iterador empieze de nuevo por si se han cambiado las direcciones de memoria
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
                else
                {
                    Client *currentClient = Client::findClient(itFds->fd);
                    if (currentClient == NULL)
                    {
                        fds.erase(itFds);
                        break;
                    }
                    try
                    {
                        if (itFds->revents & (POLLERR | POLLNVAL))
                        {
                            std::cout << "Disconection from client " << itFds->fd << std::endl;
                            removeClient(fds, itFds);
                            break;
                        }
                        /* Lectura (comando o desconexion) */
                        if ((itFds->revents & POLLIN) || (itFds->revents & POLLHUP))
                        {
                            if (currentClient->handleClient() == false)
                            {
                                std::cout << "Disconection from client " << itFds->fd << std::endl;
                                removeClient(fds, itFds);
                                break;
                            }
                        }
                        /* Una escritura por poll, solo si poll marco el socket como escribible */
                        if (itFds->revents & POLLOUT)
                        {
                            if (currentClient->flushOutgoing() == false)
                            {
                                std::cout << "Disconection from client " << itFds->fd << std::endl;
                                removeClient(fds, itFds);
                                break;
                            }
                        }
                        updatePollOut(*itFds, currentClient);
                    }
                    catch (const std::exception &e)
                    {
                        removeClient(fds, itFds);
                        std::cerr << e.what() << '\n';
                        break;
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
