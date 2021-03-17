#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <errno.h>

const int BUF_SIZE = 1024;

int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address{};
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sockfd, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrLength = sizeof(client);
    int connfd = accept(sockfd, (struct sockaddr*)&client, &client_addrLength);
    if (connfd < 0)
    {
        std::cout << "error is: " << errno;
    }
    else
    {
        char buffer[BUF_SIZE];
        memset(buffer, '\0', BUF_SIZE);
        ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
        std::cout << "got " << ret << "bytes of normal data: " << buffer;
        close(connfd);
    }
    close(sockfd);
}



