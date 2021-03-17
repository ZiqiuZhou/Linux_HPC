/*
 int listen(int sockfd, int backlog); // threshod of #socket for kernel
 influence of arg: backlog to the system.
 * */
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_pton()
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <string>
#include <string.h>

static bool STOP = false;

/* SIGTERM signal handle function,
 * stop the loop in main function
 * when trigers.
 * */
static void handle_term(int sig)
{
    STOP = true;
}

/* command lline: ./testListen 192.168.1.109 12345 5*/
int main(int argc, char* argv[]) {
    signal(SIGTERM, handle_term);
    if (argc <= 3)
    {
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    // create a socket using TCP/IPv4 protocol,
    // type = SOCK_STREAM represents TCP
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // IPv4 socket address
    struct sockaddr_in address{};
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    /* inet_pton() transform const char* type IP address
     * to IP address represented by integer with network byte order,
     * the result IP address is stored in address.sin_addr.*/
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port); // transform port with "machine byte order" to "network byte order"

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address)); // name a socket (give socket a specific address)
    assert(ret != -1);

    ret = listen(sock, backlog);
    assert(ret != -1);

    while (!STOP)
    {
        sleep(1);
    }
    close(sock);
    return 0;
}
