#include <signal.h>

#include "executor.h"
#include "network.h"
#include "notify.h"
#include "nx_model.h"
#include "util.h"
#include "video.h"
#include "xwin.h"

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    init_nx_model();
    init_video();
    init_xwin();

    listen_socket(PORT_NOTIFY, start_notify);
    listen_socket(PORT_VIDEO, start_video_capture);
    listen_socket(PORT_XWIN, start_xwin_capture);
    listen_socket(PORT_EXECUTOR, start_executor);
    broadcast_discovery_packet(PORT_UDP_BROADCAST);

    return 0;
}
