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

/*�������˷���HTTPӦ����ĳ�ĵ���HTTPӦ����������֣�1. ״̬�У�ͷ���ֶεȣ�2.�ĵ����ݡ�
  �������������ڴ�Ĳ�ͬλ�á������ĵ���������������ƴ�ӣ���ʹ��writevͬʱ������д��һ���ļ���*/

const int BUFFER_SIZE = 1024;
/* ��������HTTP״̬���״̬��Ϣ */
static const std::vector<std::string> status_line
                { "200 OK", "500 Internal server error" };

int main(int argc, char* argv[]) {
    if (argc <= 3)
    {
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    const char* file_name = argv[3]; // ������Ŀ���ļ���Ϊ��������

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
        char header_buf[BUFFER_SIZE]; // ����HTTPӦ���״̬�У�ͷ���ֶεȵĻ�����
        memset(header_buf, '\0', BUFFER_SIZE);
        char* file_buf; // ���ڴ�Ŵ�����Ŀ���ļ����ݵĻ�����
        struct stat file_stat {}; // ���ڻ�ȡĿ���ļ����ԣ������Ƿ�Ϊdir,�ļ���С��

        bool valid = true; // �ļ��Ƿ���Ч
        int len = 0; // header_bufĿǰ��ʹ�ö����ֽ�

        if (stat(file_name, &file_stat) < 0) // �������ļ�������
        {
            valid = false;
        }
        else
        {
            if (_S_IFDIR(file_stat.st_mode)) // Ŀ���ļ���dir
            {
                valid = false;
            }
            else if (file_stat.st_mode & S_IREAD) // �������ж�ȡ�ļ���Ȩ��
            {
                int fd = open(file_name, O_RDONLY);
                /* ���ļ����ݶ�������file_buf��*/
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
            /*��HTTPӦ���״̬�У�ͷ���ֶμ���head_buf*/
            ret = snprintf(header_buf, BUFFER_SIZE - 1, "HTTP/1.1", (char*)status_line[0], "\n");
            len += ret;
            ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, 
                "Content-Length", file_stat.st_size);
            len += ret;
            /* writev��header_buf��file_buf������һ��д��*/
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
