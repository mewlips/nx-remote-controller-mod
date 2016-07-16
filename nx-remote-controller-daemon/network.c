#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "network.h"
#include "util.h"
#include "video.h"
#include "notify.h"
#include "command.h"
#include "nx_model.h"

#define DISCOVERY_PACKET_SIZE 32

static int socket_connect_count;

static char *get_port_name(int port)
{
    switch (port) {
        case PORT_VIDEO:
            return "video";
        case PORT_XWIN:
            return "xwin";
        case PORT_NOTIFY:
            return "notify";
        case PORT_EXECUTOR:
            return "executor";
        case PORT_UDP_BROADCAST:
            return "discovery";
    }

    return "unknown";
}


static void *listen_socket_func(void *thread_data)
{
    ListenSocketData *listen_socket_data = (ListenSocketData *)thread_data;
    int port = listen_socket_data->port;
    OnConnect on_connect = listen_socket_data->on_connect;
    struct sockaddr_in server_addr, client_addr;
    int server_socket, client_socket;
    socklen_t len;
    pthread_t thread;
    int on = 1;

    free(listen_socket_data);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        die("socket() failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&on, (socklen_t)sizeof(on));

    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        die("bind() failed");
    }

    if (listen(server_socket, 5) == -1) {
        die("listen() failed");
    }

    len = sizeof(client_addr);

    while (true) {
        Sockets *data = (Sockets *)malloc(sizeof(Sockets));

        print_log("waiting client... port = %d (%s)", port, get_port_name(port));
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &len);
        if (client_socket == -1) {
            die("accept() failed");
        }

        print_log("client connected. port = %d (%s)", port, get_port_name(port));

        data->server_socket = server_socket;
        data->client_socket = client_socket;

        socket_connect_count++;
        if (pthread_create(&thread, NULL,
                           (void *(*)(void *))on_connect, data)) {
            die("ptherad_create() failed");
        }

        if (pthread_join(thread, NULL)) {
            die("pthread_join() failed");
        }

        if (socket_connect_count > 0) {
            socket_connect_count--;
        }

        close(client_socket);

        print_log("client closed. port = %d (%s)", port, get_port_name(port));
        print_log("connected socket count = %d", socket_connect_count);
        if (port == PORT_VIDEO) {
            notify_video_socket_closed();
        } else if (port == PORT_XWIN) {
            notify_xwin_socket_closed();
        } else if (port == PORT_EXECUTOR) {
            notify_executor_socket_closed();
        }
    }

    close(server_socket);

    return NULL;
}

void listen_socket(const int port, const OnConnect on_connect)
{
    pthread_t thread;
    ListenSocketData *thread_data
        = (ListenSocketData *)malloc(sizeof(ListenSocketData));
    thread_data->port = port;
    thread_data->on_connect = on_connect;

    if (pthread_create(&thread, NULL, listen_socket_func, thread_data)) {
        die("pthread_create() failed!");
    }

    if (pthread_detach(thread)) {
        die("pthread_detach() failed");
    }
}

void broadcast_discovery_packet(const int port)
{
    int sock;
    int broadcast_enable = 1;
    int ret;
    struct sockaddr_in sin;
    bool need_show_disconnected_msg = false;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        die("socket() failed");
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                     &broadcast_enable, sizeof(broadcast_enable));
    if (ret == -1) {
        die("setsockopt() failed");
    }

    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = (in_port_t)htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    while (true) {
        //print_log("socket_connect_count = %d", socket_connect_count);
        if (socket_connect_count == 0) {
            char msg[DISCOVERY_PACKET_SIZE] = {0,};
            char command_line[256];
            strncpy(command_line, POPUP_TIMEOUT_SH_COMMAND 
                    " 3 NXRemoteController disconnected.", 256);

            if (need_show_disconnected_msg) {
                //run_command(command_line); // FIXME
                need_show_disconnected_msg = false;
            }

            // TODO: get camera model
            // HEADER|VERSION|MODEL|
            snprintf(msg, sizeof(msg), "NX_REMOTE|1.0|%s|", get_nx_model_name()); // TODO: version
            print_log("broadcasting discovery packet... [%s]", msg);

            if (sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&sin,
                       sizeof(struct sockaddr_in)) == -1) {
                print_error("sendto() failed");
            }
        } else {
            need_show_disconnected_msg = true;
        }
        sleep(1);
    }

    if (close(sock) == -1) {
        print_error("close() failed");
    }
}
