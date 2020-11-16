#include <iostream>
#include <sys/socket.h>

#define PORT 8111

int main(int argc, char* argv[])
{
    int socket_fd;
    int ret = -1;

    struct sockaddr_in serveraddr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cout << "Failed to create socket!" << std::endl;
        exit(-1);
    }
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = PORT;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ret = connect(socket_fd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));

    if (ret == 0) {
        std::cout << "Failed to connect server!" << std::endl;
        exit(-1);
    }
    while (1) {
        send(socket_fd, sendbuf, strlen(sendbuf), 0);
    }
}