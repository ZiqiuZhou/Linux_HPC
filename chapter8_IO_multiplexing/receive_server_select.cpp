#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <cassert>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <vector>
#include <string>
#include <cerrno>
#include <fcntl.h>

/* 通过select() I/O复用让服务器端同时监听文件描述符中的可读和异常事件，
    普通事件用recv接收，异常事件用MSG_OOB标记*/
const int SIZE = 1024;

int main(int argc, char* argv[]) {
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

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct sockaddr_in client {};
    socklen_t client_addrLen = sizeof(client);
    int connfd = accept(listenfd, (struct sockaddr*)&client, &client_addrLen);
    if (connfd < 0)
    {
        std::cout << "error: " << errno;
    }

    char buf[SIZE]; // 接收数据buffer
    fd_set read_fds; // 可读事件文件描述符集合
    fd_set exception_fds; // 异常事件文件描述符集合
    FD_ZERO(&read_fds); // 清零fdset中所有位
    FD_ZERO(&exception_fds);

    while(1) {
        memset(buf, '\0', sizeof(buf));
        /*每次调用select前要重新在fd_set中设置文件描述符*/
        FD_SET(connfd, &read_fds);
        FD_SET(connfd, &exception_fds);
        select(connfd + 1, &read_fds, nullptr, &exception_fds, nullptr); // select中可写事件位NULL, 等待时间为null一直到有就绪文件描述符

        if (FD_ISSET(connfd, &read_fds))
        {
            ret = recv(connfd, buf, sizeof(buf) - 1, 0); //可读事件，将connfd上的数据读到buf
            if (ret <= 0)
            {
                break;
            }
            std::cout << "get " << ret << "bytes of normal data." << std::endl;
        }
        // 异常事件采用MSG_OOB标识的recv读取带外数据
        else if (FD_ISSET(connfd, &exception_fds))
        {
            ret = recv(connfd, buf, sizeof(buf) - 1, MSG_OOB);
            if (ret <= 0)
            {
                break;
            }
            std::cout << "get " << ret << "bytes of oob data." << std::endl;
        }
    }
    close(connfd);
    close(listenfd);
    return 0;
}
