#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

enum {
    PORT_UDP_BROADCAST = 5681,
};

#define DISCOVERY_PACKET_SIZE 32
#define MAX_NUM_CAMERAS 10

typedef struct {
    long long discovered_time;
    char ip_address[16];
    int port;
    char discovery_packet[DISCOVERY_PACKET_SIZE];
} DiscoveredCameraInfo;

extern const char *network_get_wifi_ip_address(void);
extern int network_get_discovered_cameras(char *json_buffer, size_t size);
extern void network_broadcast_discovery_packet(const int port);
extern void network_receive_discovery_packet(const int port);
extern void network_init(void);
extern void network_destroy(void);

#endif
