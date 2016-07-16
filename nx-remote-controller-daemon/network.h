#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

enum {
    PORT_NOTIFY = 5677,
    PORT_VIDEO = 5678,
    PORT_XWIN = 5679,
    PORT_EXECUTOR = 5680,
    PORT_UDP_BROADCAST = 5681,
};

typedef struct {
    int server_socket;
    int client_socket;
} Sockets;

typedef void *(*OnConnect)(Sockets *sockets);

typedef struct {
    int port;
    OnConnect on_connect;
} ListenSocketData;

extern void listen_socket(const int port, const OnConnect on_connect);
extern void broadcast_discovery_packet(const int port);

#endif
