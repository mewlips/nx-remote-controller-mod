#include <stdbool.h>

#include "command.h"
#include "executor.h"
#include "input.h"
#include "util.h"
#include "video.h"
#include "xwin.h"

#define LCD_CONTROL_SH_COMMAND "lcd_control.sh"

void *executor_start(Sockets *data)
{
    FILE *client_sock;
    int client_socket = data->client_socket;;
    char command_line[256];
    bool err = false;
    FILE *command_pipe = NULL;
    char buf[1024];
    size_t read_size;
    size_t write_size;
    unsigned long size;
    long long last_ping_time;
    int flags;

    client_sock = fdopen(data->client_socket, "r");
    if (client_sock == NULL) {
        print_error("fdopen() failed");
        err = true;
    }

    free(data);

    if (err) {
        goto error;
    }

    print_log("executor started.");

    flags = fcntl(client_socket, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(client_socket, F_SETFL, flags);

    last_ping_time = get_current_time();
    while (true) {
        if (fgets(command_line, sizeof(command_line), client_sock) == NULL) {
            if (errno == EWOULDBLOCK) {
                errno = 0;
                usleep(50*1000);
            } else {
                break;
            }
        }
        if (command_line[strlen(command_line) -1] == '\n') {
            command_line[strlen(command_line) - 1] = '\0'; // strip '\n' at end
        } else {
            continue;
        }

        if (strlen(command_line) > 0 && command_line[0] == '@') {
            // run command in background and no output return
            run_command(command_line + 1);
        } else if (strlen(command_line) > 0 && command_line[0] == '$') {
            // run command in foreground and return output
            print_log("command = %s", command_line);

            command_pipe = popen(command_line + 1, "r");
            if (command_pipe == NULL) {
                print_error("popen() failed");
                continue;
            }

            while (feof(command_pipe) == 0 && ferror(command_pipe) == 0) {
                read_size = fread(buf, 1, sizeof(buf), command_pipe);
                if (read_size == 0) {
                    break;
                } else if (read_size < 0 || read_size > sizeof(buf)) {
                    print_log("read_size = %d\n", read_size);
                    break;
                }
                while (read_size > 0) {
                    size = htonl(read_size);
                    write_size = write(client_socket, (const void *)&size, 4);
                    if (write_size == -1) {
                        print_error("write() failed!");
                        goto error;
                    }
                    write_size = write(client_socket, buf, read_size);
                    if (write_size == -1) {
                        print_error("write() failed!");
                        goto error;
                    }
                    read_size -= write_size;
                }
            }
        } else if (strncmp("inject_input=", command_line, 13) == 0) {
            input_inject(command_line + 13);
        } else if (strncmp("vfps=", command_line, 5) == 0) {
            video_set_fps(atoi(command_line + 5));
        } else if (strncmp("xfps=", command_line, 5) == 0) {
            xwin_set_fps(atoi(command_line + 5));
        } else if (strncmp("lcd=", command_line, 4) == 0) {
            char command[256];
            char *arg = command_line + 4;
            snprintf(command, sizeof(command), "%s/%s %s",
                     get_app_path(), LCD_CONTROL_SH_COMMAND, arg);
            //print_log("command = %s", command);
            system(command);
        } else if (strncmp("ping", command_line, 4) == 0) {
            last_ping_time = get_current_time();
        }

        // EOF
        size = 0;
        write_size = write(client_socket, (const void *)&size, 4);
        if (write_size == -1) {
            print_error("write() failed!");
            goto error;
        }

        if (command_pipe != NULL && pclose(command_pipe) == -1) {
            //print_error("pclose() failed!");
            command_pipe = NULL;
        }

        if (last_ping_time < get_current_time() - PING_TIMEOUT_MS) {
            print_log("executor ping not reached.");
            goto error;
        }
    }

error:
    if (command_pipe != NULL) {
        if (pclose(command_pipe) == -1) {
            //print_error("pclose() failed!");
        }
    }

    print_log("executor finished.");
    return NULL;
}


