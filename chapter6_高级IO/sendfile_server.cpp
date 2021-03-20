#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <vector>
#include <string>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/sendfile.h>

/*与writev相比，sendfile 没有为待发送目标文件分配buffer，也无需读取文件操作将内容写入buffer,效率很高*/

int main(int argc, char* argv[]) {
    if (argc <= 3)
    {
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    const char* file_name = argv[3]; // 待传输目标文件作为参数传入

    struct sockaddr_in server_address {};
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
        int filefd = open(file_name, O_RDONLY);
        assert(filefd > 0);
        struct stat stat_buf;
        fstat(filefd, &stat_buf);

        sendfile(connfd, filefd, nullptr, stat_buf.st_size); // 第一个参数必须是socket, 第二个参数必须是待发送文件(不可是socket)
        close(connfd);
    }
    close(sock);
    return 0;
}
