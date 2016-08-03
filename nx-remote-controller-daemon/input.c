#include <stdio.h>

#include "command.h"
#include "input.h"
#include "util.h"

#define NX_INPUT_INJECTOR_COMMAND "nx-input-injector"

static FILE *s_inject_input_pipe = NULL;

void input_init(void)
{
    char command_line[256];

    snprintf(command_line, sizeof(command_line), "%s %s",
             get_chroot_command(), NX_INPUT_INJECTOR_COMMAND);
    s_inject_input_pipe = popen(command_line, "w");
    if (s_inject_input_pipe == NULL) {
        print_error("popen() failed");
    }
}

void input_destroy(void)
{
    if (s_inject_input_pipe != NULL) {
        if (pclose(s_inject_input_pipe) == -1) {
            //print_error("pclose() failed!");
        }
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
