#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "util.h"

static char s_app_path[256];
static char s_chroot_command[256];

void run_command(char *command_line)
{
    pid_t pid = fork();
    if (pid == 0) { // child
        print_log("execvp(), %s", command_line);
#define MAX_ARGS 63
        int argc = 0;
        char *argv[MAX_ARGS + 1];
        char *p = strtok(command_line, " ");
        while (p && argc < MAX_ARGS) {
            argv[argc++] = p;
            p = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        execvp(argv[0], argv);
    } else if (pid > 0) {
        // parent. do nothing
    } else {
        print_error("fork() failed!");
    }
}

const char *get_app_path(void)
{
    ssize_t bytes;

    if (s_app_path[0] != '\0') {
        return s_app_path;
    }

    bytes = readlink("/proc/self/cwd", s_app_path, sizeof(s_app_path) - 1);
    if (bytes == -1) {
        print_error("readlink() failed.");
        return NULL;
    }
    s_app_path[bytes] = '\0';

    return s_app_path;
}

const char *get_chroot_command(void)
{
    if (s_chroot_command[0] == '\0') {
        snprintf(s_chroot_command, sizeof(s_chroot_command),
                 "chroot %s/tools ", get_app_path());
    }

    return s_chroot_command;
}
