#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "command.h"
#include "util.h"

static unsigned long s_di_camera_app_window_id;

void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

long long get_current_time(void)
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

unsigned long get_di_camera_app_window_id(void)
{
    if (s_di_camera_app_window_id == 0) {
        char command[256], buf[256];
        FILE *command_pipe;

        snprintf(command, sizeof(command),
                 "%s xdotool search --class di-camera-app",
                 get_chroot_command());
        command_pipe = popen(command, "r");
        if (command_pipe == NULL) {
            print_error("popen() failed!");
            return 0;
        }

        if (fgets(buf, sizeof(buf), command_pipe) != NULL) {
            s_di_camera_app_window_id = strtoul(buf, NULL, 10);
        }

        pclose(command_pipe);
    }

    return  s_di_camera_app_window_id;
}
