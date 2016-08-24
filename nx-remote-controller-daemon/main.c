#include <signal.h>

#include "api_server.h"
#include "input.h"
#include "liveview.h"
#include "network.h"
#include "nx_model.h"
#include "osd.h"
#include "util.h"

int main(int argc, char **argv)
{
    const char *ip_addr;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    nx_model_init();
    input_init();

    while (true) {
        ip_addr = network_get_wifi_ip_address();
        if (ip_addr == NULL) {
            sleep(1);
            continue;
        }

        liveview_init();
        osd_init();
        network_init();

        network_broadcast_discovery_packet(PORT_UDP_BROADCAST);
        network_receive_discovery_packet(PORT_UDP_BROADCAST);

        api_server_run();

        liveview_destroy();
        osd_destroy();
        network_destroy();

        // wait for threads terimantion
        sleep(3);
    }

    input_destroy();

    return 0;
}
