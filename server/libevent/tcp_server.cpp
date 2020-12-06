#include <iostream>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#define PORT 8111

void on_read_cb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *input = NULL;
    struct evbuffer *output = NULL;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);
    evbuffer_add_buffer(output, input);
}

void on_accept_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *ctx)
{
    struct event_base *base = NULL;
    struct bufferevent *bev = NULL;
    base = evconnlistener_get_base(listener);
    bev = bufferevent_socket_new(base, fd, 0);

    bufferevent_enable(bev, EV_READ | EV_WRITE);
    bufferevent_setcb(bev, on_read_cb, NULL, NULL, NULL);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serveraddr;
    struct event_base *base = NULL;
    struct evconnlistener *listener = NULL;

    base = event_base_new();

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    listener = evconnlistener_new_bind(base, on_accept_cb, NULL, LEV_OPT_REUSEABLE, 10, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
    event_base_dispatch(base);
    return 0;
}