#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8111
#define MESSAGE_LEN 1024
#define MAX_EVENTS 20
#define TIMEOUT 500

int main(int argc, char* argv[])
{
    int ret = -1;
    int socket_fd, accept_fd;
    int on = 1;
    int backlog = 10;

    struct sockaddr_in localaddr, remoteaddr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    char in_buff[MESSAGE_LEN] = {
        0,
    };

    int epoll_fd;
    struct epoll_event ev, events[MAX_EVENTS];
    int event_number;
    int flags;

    // create socket
    if (socket_fd == -1) {
        std::cout << "Failed to create socket!" << std::endl;
        exit(-1);
    }

    flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
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

    epoll_fd = epoll_create(256);

    ev.events = EPOLLIN;
    ev.data.fd = socket_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);

    // accept socket
    for (;;) {
        event_number = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        for (int i = 0; i < event_number; i++) {
            if (events[i].data.fd = socket_fd) {
                socklen_t addr_len = sizeof(struct sockaddr);
                accept_fd = accept(socket_fd, (struct sockaddr*)&remoteaddr, &addr_len);

                flags = fcntl(accept_fd, F_GETFL, 0);
                fcntl(accept_fd, F_SETFL, flags | O_NONBLOCK);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = accept_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_fd, &ev);
            }
        }

        // receive data
        for (;;) {
            memset(in_buff, 0, MESSAGE_LEN);
            ret = recv(accept_fd, (void*)in_buff, MESSAGE_LEN, 0);
            if (ret == 0) {
                break;
            }

            // log data
            std::cout << "recv:" << in_buff << std::endl;
            send(accept_fd, (void*)in_buff, MESSAGE_LEN, 0);
        }
        close(accept_fd);
    }
    close(socket_fd);
    return 0;
}