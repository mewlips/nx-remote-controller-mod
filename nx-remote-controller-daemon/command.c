#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "util.h"

static char s_app_path[256];
static char s_chroot_command[256];

void run_command(char *command_line)
{
    print_log("command_line = %s", command_line);
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
    const char *app_path;

    if (s_app_path[0] != '\0') {
        return s_app_path;
    }

    app_path = getenv("APP_PATH");
    if (app_path == NULL) {
        bytes = readlink("/proc/self/cwd", s_app_path, sizeof(s_app_path) - 1);
        if (bytes == -1) {
            print_error("readlink() failed.");
            return NULL;
        }
        s_app_path[bytes] = '\0';
    } else {
        snprintf(s_app_path, sizeof(s_app_path), "%s", app_path);
    }

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

int systemf(const char *fmt, ...)
{
    char command[256];
    va_list arg_ptr;

    va_start(arg_ptr, fmt);
    vsnprintf(command, sizeof(command), fmt, arg_ptr);
    va_end(arg_ptr);

    return system(command);
}
