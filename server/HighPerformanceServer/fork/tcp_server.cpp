#include <iostream>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8111
#define MESSAGE_LEN 1024

int main(int argc, char* argv[])
{
    int ret = -1;
    int socket_fd, accept_fd;
    int on = 1;
    pid_t pid;
    int backlog = 10;

    struct sockaddr_in localaddr, remoteaddr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    char in_buff[MESSAGE_LEN] = {
        0,
    };

    // create socket
    if (socket_fd == -1) {
        std::cout << "Failed to create socket!" << std::endl;
        exit(-1);
    }

    // set socket option
    ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        std::cout << "Failed to set socket options!" << std::endl;
    }

    localaddr.sin_family = AF_INET;
    localaddr.sin_port = PORT;
    localaddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(localaddr.sin_zero), 8);

    // bind socket
    ret = bind(socket_fd, (struct sockaddr*)&localaddr, sizeof(struct sockaddr));
    if (ret == -1) {
        std::cout << "Failed to bind addr!" << std::endl;
        exit(-1);
    }

    // listen socket
    ret = listen(socket_fd, backlog);
    if (ret == -1) {
        std::cout << "Failed to listen addr!" << std::endl;
        exit(-1);
    }

    // accept socket
    for (;;) {
        socklen_t addr_len = sizeof(struct sockaddr);
        accept_fd = accept(socket_fd, (struct sockaddr*)&remoteaddr, &addr_len);

        pid = fork();
        if (pid == 0) {
            for (;;) {
                ret = recv(accept_fd, (void*)in_buff, MESSAGE_LEN, 0);
                if (ret == 0) {
                    break;
                }

                // log data
                std::cout << "recv:" << in_buff << std::endl;
                send(accept_fd, (void*)in_buff, MESSAGE_LEN, 0);
            }
        }
        close(accept_fd);
    }
    if (pid != 0) {
        close(socket_fd);
    }
    return 0;
}