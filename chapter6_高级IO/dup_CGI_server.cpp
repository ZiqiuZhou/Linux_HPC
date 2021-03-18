#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <cerrno>

int main(int argc, char* argv[]) {
    if (argc <= 2)
    {
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_address{};
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr*)&server_address, sizeof(server_address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client {};
    socklen_t client_addrLen = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrLen);
    if (connfd < 0)
    {
        std::cout << "error: " << errno;
    }
    else
    {
        close(STDOUT_FILENO); // close file descriptor (fd = 1) of standard output
        dup(connfd); // dup client fd, here dup tp the min fd, which is STDOUT_FILENO
        std::cout << "abcd." << std::endl; // standard output can directly send to client socket
        close(connfd);
    }
    close(server_address);
    return 0;
}
