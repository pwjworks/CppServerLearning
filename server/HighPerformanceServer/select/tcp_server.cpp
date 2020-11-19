#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8111
#define MESSAGE_LEN 1024
#define FD_SIZE 1024

int main(int argc, char* argv[])
{
    int ret = -1;
    int socket_fd, accept_fd;
    int on = 1;
    int backlog = 10;
    int flags;
    int max_fd = -1;
    int curpos = -1;
    int events = 0;
    fd_set fd_sets;
    int accept_fds[FD_SIZE] = {
        -1,
    };

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

    flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    max_fd = socket_fd;
    // listen socket
    ret = listen(socket_fd, backlog);
    if (ret == -1) {
        std::cout << "Failed to listen addr!" << std::endl;
        exit(-1);
    }

    // accept socket
    for (;;) {
        FD_ZERO(&fd_sets);
        FD_SET(socket_fd, &fd_sets);

        for (int i = 0; i < FD_SIZE; i++) {
            if (accept_fds[i] != -1) {
                if (accept_fds[i] > max_fd) {
                    max_fd = accept_fds[i];
                }
                FD_SET(accept_fds[i], &fd_sets);
            }
        }

        events = select(max_fd + 1, &fd_sets, NULL, NULL, NULL);
        if (events < 0) {
            std::cout << "Failed to use select!" << std::endl;
            break;
        } else if (events == 0) {
            std::cout << "timeout...!" << std::endl;
            continue;
        } else if (events) {
            if (FD_ISSET(socket_fd, &fd_sets)) {
                for (int i = 0; i < FD_SIZE; i++) {
                    if (accept_fds[i] == -1) {
                        curpos = i;
                        break;
                    }
                }
                socklen_t addr_len = sizeof(struct sockaddr);
                accept_fd = accept(socket_fd, (struct sockaddr*)&remoteaddr, &addr_len);
                flags = fcntl(accept_fd, F_GETFL, 0);
                fcntl(accept_fd, F_SETFL, flags | O_NONBLOCK);

                accept_fds[curpos] = accept_fd;
            }

            for (int i = 0; i < FD_SIZE; i++) {
                if (accept_fds[i] != -1 && FD_ISSET(accept_fds[i], &fd_sets)) {
                    memset(in_buff, 0, MESSAGE_LEN);
                    ret = recv(accept_fds[i], (void*)in_buff, MESSAGE_LEN, 0);
                    if (ret == 0) {
                        close(accept_fds[i]);
                        break;
                    }

                    // log data
                    std::cout << "recv:" << in_buff << std::endl;
                    send(accept_fds[i], (void*)in_buff, MESSAGE_LEN, 0);
                }
            }
        }
    }
    close(socket_fd);
    return 0;
}