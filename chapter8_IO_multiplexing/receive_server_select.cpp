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

/* ͨ��select() I/O�����÷�������ͬʱ�����ļ��������еĿɶ����쳣�¼���
    ��ͨ�¼���recv���գ��쳣�¼���MSG_OOB���*/
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

    char buf[SIZE]; // ��������buffer
    fd_set read_fds; // �ɶ��¼��ļ�����������
    fd_set exception_fds; // �쳣�¼��ļ�����������
    FD_ZERO(&read_fds); // ����fdset������λ
    FD_ZERO(&exception_fds);

    while(1) {
        memset(buf, '\0', sizeof(buf));
        /*ÿ�ε���selectǰҪ������fd_set�������ļ�������*/
        FD_SET(connfd, &read_fds);
        FD_SET(connfd, &exception_fds);
        select(connfd + 1, &read_fds, nullptr, &exception_fds, nullptr); // select�п�д�¼�λNULL, �ȴ�ʱ��Ϊnullһֱ���о����ļ�������

        if (FD_ISSET(connfd, &read_fds))
        {
            ret = recv(connfd, buf, sizeof(buf) - 1, 0); //�ɶ��¼�����connfd�ϵ����ݶ���buf
            if (ret <= 0)
            {
                break;
            }
            std::cout << "get " << ret << "bytes of normal data." << std::endl;
        }
        // �쳣�¼�����MSG_OOB��ʶ��recv��ȡ��������
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
