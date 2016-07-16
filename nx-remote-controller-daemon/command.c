#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "command.h"
#include "util.h"

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

