#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>
#include <string.h>

int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address{}; // TCP
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    sleep(20); //sleep 20s to wait for client connection
    struct sockaddr_in client {}; // store client address information which accepted to connect
    socklen_t client_addrlength = sizeof(client);
    int connectFd = accept(sock, (struct sockaddr*) &client, &client_addrlength); // accept the connection
    if (connectFd < 0)
    {
        std::cout << "acception error.";
    }
    else
    {
        /*print IP addr & port of connected client */
        char remote[INET_ADDRSTRLEN]; // store ip address represent as string
        std::cout << " connected with ip: " << inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN) << std::endl;
        std::cout << " port: " << ntohs(client.sin_port);
        close(connectFd);
    }
    close(sock);
    return 0;
}



