#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "network.h"
#include "nx_model.h"
#include "util.h"
#include "version.h"


static bool s_stopped;
static char s_wifi_ip_address[32];
static DiscoveredCameraInfo s_cameras[MAX_NUM_CAMERAS];
static pthread_mutex_t s_mutex;

const char *network_get_wifi_ip_address(void)
{
    int sock;
    struct ifreq ifr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "mlan0", IFNAMSIZ-1);
    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        print_error("ioctl() failed");
        s_wifi_ip_address[0] = '\0';
        return NULL;
    }

    close(sock);

    strncpy(s_wifi_ip_address,
            inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
            sizeof(s_wifi_ip_address));

    return s_wifi_ip_address;
}

static void put_camera(const char *ip_addr, int port, const char *packet)
{
    DiscoveredCameraInfo *info = NULL;
    int i, oldest_index;
    long long current_time = get_current_time();
    long long oldest_time = current_time;

    if (ip_addr == NULL || packet == NULL) {
        return;
    }

    pthread_mutex_lock(&s_mutex);
    oldest_index = -1;
    for (i = 0; i < ARRAY_SIZE(s_cameras); i++) {
        if (s_cameras[i].port == port) {
            info = &s_cameras[i];
            break;
        }
        if (oldest_time > s_cameras[i].discovered_time) {
            oldest_time = s_cameras[i].discovered_time;
            oldest_index = i;
        }
    }

    if (info == NULL && oldest_index != -1) {
        info = &s_cameras[oldest_index];
    }

    if (info != NULL) {
        info->discovered_time = current_time;
        strncpy(info->ip_address, ip_addr, sizeof(info->ip_address));
        info->port = port;
        strncpy(info->discovery_packet, packet, sizeof(info->discovery_packet));
        //print_log("info = %p", info);
    } else {
        print_log("fatal error!!!");
    }
    pthread_mutex_unlock(&s_mutex);
}

int network_get_discovered_cameras(char *json_buffer, size_t size)
{
    int i, j;
    DiscoveredCameraInfo *info;
    long long current_time = get_current_time();


    j = snprintf(json_buffer, size, "["); // start json array

    pthread_mutex_lock(&s_mutex);
    for (i = 0; i < ARRAY_SIZE(s_cameras); i++) {
        info = &s_cameras[i];
        if (info->discovered_time > current_time - 2000) {
            j += snprintf(json_buffer + j, size - j,
                          "{\"ip\":\"%s\",\"packet\":\"%s\"},",
                          info->ip_address, info->discovery_packet);
        }
    }
    if (j != 1) { // not empty array
        j -= 1;
    }
    pthread_mutex_unlock(&s_mutex);
    j = snprintf(json_buffer + j, size - j, "]"); // end json array

    //print_log("json = %s", json_buffer);

    if (j > size) {
        print_log("too small json buffer");
        return -1;
    }

    return j;
}

static void *broadcast_discovery_packet_func(void *data)
{
    int port = (int)data;
    int sock;
    int broadcast_enable = 1;
    int ret;
    struct sockaddr_in sin;

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

    while (!s_stopped) {
        char msg[DISCOVERY_PACKET_SIZE] = {0,};

        // HEADER|VERSION|MODEL|FW_VERSION|
        snprintf(msg, sizeof(msg), "NX_REMOTE|%s|%s|%s|",
                 VERSION, get_nx_model_name(), get_nx_model_version());
        print_log("broadcasting discovery packet... [%s]", msg);

        if (sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&sin,
                   sizeof(struct sockaddr_in)) == -1) {
            print_error("sendto() failed");
        }
        sleep(1);
    }

    if (close(sock) == -1) {
        print_error("close() failed");
    }

    print_log("broadcast_discovery_packet_func thread terminated.");

    return NULL;
}

static void *receive_discovery_packet_func(void *data)
{
    int port = (int)data;
    int sock;
    char msg[DISCOVERY_PACKET_SIZE];
    struct sockaddr_in sin;
    int status;
    size_t sin_len;
    struct timeval tv;
    int yes = 1;

    sin_len = sizeof(struct sockaddr_in);
    memset(&sin, 0, sin_len);
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        die("socket() failed");
    }

    tv.tv_sec = 2;
    tv.tv_usec = 0;
    status = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (status == -1) {
        die("setsockopt() failed!");
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        die("setsockopt() failed!");
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;

    status = bind(sock, (struct sockaddr *)&sin, sin_len);
    if (status == -1) {
        die("bind() failed!");
    }

    while (!s_stopped) {
        memset(msg, 0, sizeof(msg));
        status = recvfrom(sock, msg, sizeof(msg), 0, (struct sockaddr *)&sin, &sin_len);
        if (status != -1) {
            put_camera(inet_ntoa(sin.sin_addr), htons(sin.sin_port), msg);
        } else {
            sleep(1);
        }
    }

    if (shutdown(sock, 2) == -1) {
        print_error("shutdown() failed");
    }
    if (close(sock) == -1) {
        print_error("close() failed");
    }

    print_log("network_receive_discovery_packet thread terminated.");

    return NULL;
}

void network_broadcast_discovery_packet(int port)
{
    pthread_t thread;

    if (pthread_create(&thread, NULL, broadcast_discovery_packet_func,
                       (void *)port)) {
        die("pthread_create() failed!");
    }

    if (pthread_detach(thread)) {
        die("pthread_detach() failed");
    }
}

void network_receive_discovery_packet(int port)
{
    pthread_t thread;

    if (pthread_create(&thread, NULL, receive_discovery_packet_func,
                       (void *)port)) {
        die("pthread_create() failed!");
    }

    if (pthread_detach(thread)) {
        die("pthread_detach() failed");
    }
}

void network_init(void)
{
    s_stopped = false;
    pthread_mutex_init(&s_mutex, NULL);
}

void network_destroy(void)
{
    s_stopped = true;
    pthread_mutex_destroy(&s_mutex);
}
