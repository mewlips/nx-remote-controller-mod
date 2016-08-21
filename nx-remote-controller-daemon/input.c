#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "input.h"
#include "nx_model.h"
#include "status.h"
#include "util.h"

#define NX_INPUT_INJECTOR_COMMAND "nx-input-injector"
#define XEV_NX_COMMAND "xev-nx"

static FILE *s_inject_input_pipe = NULL;
static int s_notify_client_socket = -1;
static bool s_stopped = false;

static void process_mode(const char *key)
{
    if (!strcmp(key, "F4") ||
        !strcmp(key, "XF86Send")) {
        set_dial_mode(MODE_SCENE);
    } else if (!strcmp(key, "F6") ||
               !strcmp(key, "XF86Reply")) {
        set_dial_mode(MODE_SMART);
    } else if (!strcmp(key, "F7") ||
               !strcmp(key, "XF86MailForward")) {
        set_dial_mode(MODE_P);
    } else if (!strcmp(key, "F8") ||
               !strcmp(key, "XF86Save")) {
        set_dial_mode(MODE_A);
    } else if (!strcmp(key, "F9") ||
               !strcmp(key, "XF86Documents")) {
        set_dial_mode(MODE_S);
    } else if (!strcmp(key, "F10") ||
               !strcmp(key, "XF86Battery")) {
        set_dial_mode(MODE_M);
    } else if (!strcmp(key, "KP_Home") ||
               !strcmp(key, "XF86WLAN")) {
        set_dial_mode(MODE_C1);
    } else if (!strcmp(key, "Scroll_Lock") ||
               !strcmp(key, "XF86Bluetooth")) {
        set_dial_mode(MODE_C2);
    } else if (!strcmp(key, "F1") ||
               !strcmp(key, "XF86KbdBrightnessDown")) {
        if (is_old_nx_model()) {
            set_dial_mode(MODE_NX300_LENS_PRIORITY);
        } else {
            set_dial_mode(MODE_SAS);
        }
    } else if (!strcmp(key, "F3")) {
        set_dial_mode(MODE_NX300_WIFI);
    }
}

static void process_key(const char *line)
{
    if (!strncmp(line, "keyup ", 6)) {
        process_mode(line + 6);
    }
}

static void *catch_inputs(void *data)
{
    FILE *xev_pipe = NULL;
    char command[256];
    char buf[256];
    pid_t xev_pid = 0;

    snprintf(command, sizeof(command),
            "%s %s -p -id "
            "\"$(%s xdotool search --class di-camera-app)\"",
            get_chroot_command(), XEV_NX_COMMAND, get_chroot_command());
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

    while (!s_stopped) {
        if (fgets(buf, sizeof(buf), xev_pipe) != NULL) {
            if (s_notify_client_socket != -1) {
                if (write(s_notify_client_socket, buf, strlen(buf)) == -1) {
                    print_error("write() failed!");
                }
            }
            buf[strlen(buf) - 1] = '\0';
            if (!strncmp(buf, "key", 3)) {
                process_key(buf);
            }
        } else {
            break;
        }
    }

error:
    if (xev_pid != 0 && kill(xev_pid, SIGKILL) == -1) {
        print_error("kill() failed!");
    }
    if (xev_pipe != NULL && pclose(xev_pipe) == -1) {
        //print_error("pclose() failed!");
    }

    print_log("catch_inputs thread terminated.");

    return NULL;
}

void input_init(void)
{
    pthread_t thread;
    char command_line[256];

    s_stopped = false;

    snprintf(command_line, sizeof(command_line), "%s %s",
             get_chroot_command(), NX_INPUT_INJECTOR_COMMAND);
    s_inject_input_pipe = popen(command_line, "w");
    if (s_inject_input_pipe == NULL) {
        print_error("popen() failed");
    }

    if (pthread_create(&thread, NULL, catch_inputs, NULL)) {
        die("pthread_create() failed!");
    }

    if (pthread_detach(thread)) {
        die("pthread_detach() failed!");
    }
}

void input_destroy(void)
{
    s_stopped = true;

    if (s_inject_input_pipe != NULL) {
        if (pclose(s_inject_input_pipe) == -1) {
            //print_error("pclose() failed!");
        }
        print_log("s_inject_input_pipe closed.");
    }
}

void input_inject(const char *command)
{
    if (s_inject_input_pipe != NULL) {
        print_log("command = %s", command);
        fprintf(s_inject_input_pipe, "%s\n", command);
        fflush(s_inject_input_pipe);
    }
}

void input_set_notify_socket(int client_socket)
{
    s_notify_client_socket = client_socket;
}

void input_remove_notify_socket(void)
{
    s_notify_client_socket = -1;
}
