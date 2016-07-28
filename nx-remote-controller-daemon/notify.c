#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "notify.h"
#include "nx_model.h"
#include "util.h"

#define HEVC_STATE_UNKNOWN (-1)
#define HEVC_STATE_OFF 0
#define HEVC_STATE_ON  1

static bool s_video_socket_closed_notify;
static bool s_xwin_socket_closed_notify;
static bool s_executor_socket_closed_notify;
static bool s_evf_start_notify;
static bool s_evf_end_notify;

void notify_video_socket_closed(void)
{
    s_video_socket_closed_notify = true;
}

void notify_xwin_socket_closed(void)
{
    s_xwin_socket_closed_notify = true;
}

void notify_executor_socket_closed(void)
{
    s_executor_socket_closed_notify = true;
}

void notify_evf_start(void)
{
    s_evf_start_notify = true;
}

void notify_evf_end(void)
{
    s_evf_end_notify = true;
}

void *start_notify(Sockets *sockets)
{
    int client_socket = sockets->client_socket;
    FILE *xev_pipe = NULL;
    char command[256];
    char buf[256];
    int xev_fd;
    int flags;
    size_t write_size;
    pid_t xev_pid = 0;
    FILE *hevc = NULL;
    int hevc_state = HEVC_STATE_UNKNOWN;
    char *line;
    int count = 0;

    free(sockets);

    if (is_new_nx_model()) {
        hevc = fopen("/sys/kernel/debug/pmu/hevc/state", "r");
        if (hevc == NULL) {
            print_error("fopen() failed");
            goto error;
        }
    }

    snprintf(command, sizeof(command),
            "%s xev-nx -p -id "
            "\"$(%s xdotool search --class di-camera-app)\"",
            get_chroot_command(), get_chroot_command());
    //print_log("xev-nx command = %s", command);
    xev_pipe = popen(command, "r");
    if (xev_pipe == NULL) {
        print_error("popen() failed!");
        goto error;
    }

    if (fgets(buf, sizeof(buf), xev_pipe) != NULL) {
        sscanf(buf, "%d\n", &xev_pid);
        print_log("xev-nx pid = %d", xev_pid);
    } else {
        print_log("failed get xev-nx pid.");
        goto error;
    }

    xev_fd = fileno(xev_pipe);
    flags = fcntl(xev_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(xev_fd, F_SETFL, flags);

    while (true) {
        if (is_new_nx_model()) {
            // hevc check
            clearerr(hevc);
            rewind(hevc);
            memset(buf, 0, sizeof(buf));
            fread(buf, 1, sizeof(buf), hevc);
            if (ferror(hevc) != 0) {
                print_log("ferror()");
            } else if (feof(hevc) != 0) {
                //print_log("read_size = %d, buf = %s", read_size, buf);
                if (strncmp(buf, "on", 2) == 0) {
                    if (hevc_state != HEVC_STATE_ON) {
                        hevc_state = HEVC_STATE_ON;
                        write_size = write(client_socket, "hevc=on\n", 8);
                        if (write_size == -1) {
                            print_error("write() failed!");
                            goto error;
                        }
                    }
                } else if (strncmp(buf, "off", 3) == 0) {
                    if (hevc_state != HEVC_STATE_OFF) {
                        hevc_state = HEVC_STATE_OFF;
                        write_size = write(client_socket, "hevc=off\n", 9);
                        if (write_size == -1) {
                            print_error("write() failed!");
                            goto error;
                        }
                    }
                }
            }
        }

        // xev-nx
        line = fgets(buf, sizeof(buf), xev_pipe);
        if (line == NULL) {
            if (errno == EWOULDBLOCK) {
                errno = 0;
                usleep(100*1000);
            } else {
                print_log("failed to read from xev_pipe.");
                break;
            }
        } else {
            write_size = write(client_socket, buf, strlen(buf));
            if (write_size == -1) {
                print_log("write() failed. write_size = %d", write_size);
                break;
            }
            //print_log("buf = %s", buf);
        }

        if (s_video_socket_closed_notify) {
            char msg[] = "socket_closed=video\n";
            write_size = write(client_socket, msg, strlen(msg));
            if (write_size == -1) {
                print_log("write() failed!");
                break;
            }
            s_video_socket_closed_notify = false;
        }

        if (s_xwin_socket_closed_notify) {
            char msg[] = "socket_closed=xwin\n";
            write_size = write(client_socket, msg, strlen(msg));
            if (write_size == -1) {
                print_log("write() failed!");
                break;
            }
            s_xwin_socket_closed_notify = false;
        }

        if (s_executor_socket_closed_notify) {
            char msg[] = "socket_closed=executor\n";
            write_size = write(client_socket, msg, strlen(msg));
            if (write_size == -1) {
                print_log("write() failed!");
                break;
            }
            s_executor_socket_closed_notify = false;
        }

        if (s_evf_start_notify) {
            char msg[] = "evf=on\n";
            write_size = write(client_socket, msg, strlen(msg));
            if (write_size == -1) {
                print_log("write() failed!");
                break;
            }
            s_evf_start_notify = false;
        }

        if (s_evf_end_notify) {
            char msg[] = "evf=off\n";
            write_size = write(client_socket, msg, strlen(msg));
            if (write_size == -1) {
                print_log("write() failed!");
                break;
            }
            s_evf_end_notify = false;
        }


        // send ping
        if (count % 10 == 0) {
            write_size = write(client_socket, "ping\n", 5);
            if (write_size == -1) {
                print_log("write() failed. ping failed");
                break;
            }
            count = 0;
        }
        count++;
    }

error:
    if (is_new_nx_model()) {
        if (hevc != NULL && fclose(hevc)) {
            print_error("fclose() failed!");
        }
    }
    if (xev_pid != 0 && kill(xev_pid, SIGKILL) == -1) {
        print_error("kill() failed!");
    }
    if (xev_pipe != NULL && pclose(xev_pipe) == -1) {
        //print_error("pclose() failed!");
    }

    print_log("notify finished.");

    return NULL;
}


