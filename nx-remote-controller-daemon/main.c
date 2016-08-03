#include <signal.h>

#include "api_server.h"
#include "executor.h"
#include "input.h"
#include "liveview.h"
#include "network.h"
#include "notify.h"
#include "nx_model.h"
#include "osd.h"
#include "util.h"
#include "video.h"
#include "xwin.h"

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    nx_model_init();
    video_init();
    xwin_init();
    liveview_init();
    osd_init();
    input_init();

    listen_socket(PORT_NOTIFY, notify_start);
    listen_socket(PORT_VIDEO, video_start_capture);
    listen_socket(PORT_XWIN, xwin_start_capture);
    listen_socket(PORT_EXECUTOR, executor_start);
    broadcast_discovery_packet(PORT_UDP_BROADCAST);

    api_server_run();

    liveview_destroy();
    osd_destroy();
    input_destroy();

    return 0;
}
