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

/*服务器端发送HTTP应答传输某文档。HTTP应答包含两部分，1. 状态行，头部字段等；2.文档内容。
  两部分内容在内存的不同位置。传输文档无需两部分内容拼接，可使用writev同时将它们写到一个文件中*/

const int BUFFER_SIZE = 1024;
/* 定义两种HTTP状态码和状态信息 */
static const std::vector<std::string> status_line
                { "200 OK", "500 Internal server error" };

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
        char header_buf[BUFFER_SIZE]; // 保存HTTP应答的状态行，头部字段等的缓冲区
        memset(header_buf, '\0', BUFFER_SIZE);
        char* file_buf; // 用于存放待传输目标文件内容的缓冲区
        struct stat file_stat {}; // 用于获取目标文件属性，日入是否为dir,文件大小等

        bool valid = true; // 文件是否有效
        int len = 0; // header_buf目前已使用多少字节

        if (stat(file_name, &file_stat) < 0) // 待传输文件不存在
        {
            valid = false;
        }
        else
        {
            if (_S_IFDIR(file_stat.st_mode)) // 目标文件是dir
            {
                valid = false;
            }
            else if (file_stat.st_mode & S_IREAD) // 服务器有读取文件的权限
            {
                int fd = open(file_name, O_RDONLY);
                /* 将文件内容读到缓存file_buf中*/
                file_buf = new char[file_stat.st_size + 1];
                memset(file_buf, '\0', file_stat.st_size + 1);
                if (read(fd, file_buf, file_stat.st_size) < 0)
                {
                    valid = false;
                }
            }
            else
            {
                valid = false;
            }
        }
        if (valid)
        {
            /*将HTTP应答的状态行，头部字段加入head_buf*/
            ret = snprintf(header_buf, BUFFER_SIZE - 1, "HTTP/1.1", (char*)status_line[0], "\n");
            len += ret;
            ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, 
                "Content-Length", file_stat.st_size);
            len += ret;
            /* writev将header_buf和file_buf的内容一并写入*/
            struct iovec iv[2];
            iv[0].iov_base = header_buf;
            iv[0].iov_len = strlen(header_buf);
            iv[1].iov_base = file_buf;
            iv[1].iov_len = file_stat.st_size;
            ret =  writev(connfd, iv, 2);
        }
        else /* if valid == false*/
        {
            ret = snprintf(header_buf, BUFFER_SIZE - 1, "HTTP/1.1", (char*)status_line[1], "\n");
            send(connfd, header_buf, strlen(header_buf), 0);
        }
        close(connfd);
        delete[] file_buf;
    }
    close(sock);
    return 0;
}
