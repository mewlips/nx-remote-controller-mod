#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "nx_model.h"
#include "util.h"
#include "mongoose.h"

static bool s_stopped;
static char s_png_path[128];

static void *cap_osd_thread(void *data)
{
    char osd_cap_command[128];
    FILE *command_pipe;
    char line[128];

    snprintf(osd_cap_command, sizeof(osd_cap_command), "%s osd_cap.sh %s",
             get_chroot_command(), get_nx_model_name());

    //print_log("osd_cap_command = %s", osd_cap_command);

    while (!s_stopped) {
        command_pipe = popen(osd_cap_command, "r");
        if (command_pipe == NULL) {
            print_error("popen() failed");
            // TODO: server internal error
            return NULL;
        }

        while (fgets(line, sizeof(line), command_pipe) != NULL) {
            line[strlen(line) - 1] = '\0';
        }
        pclose(command_pipe);

        // TODO: mutex
        snprintf(s_png_path, sizeof(s_png_path), "%s/tools%s", get_app_path(), line);
        print_log("png = %s", s_png_path);
    }

    return NULL;
}

void init_osd(void)
{
    pthread_t thread;

    if (pthread_create(&thread, NULL, cap_osd_thread, NULL)) {
        print_error("pthread_create() failed");
        return;
    }
    if (pthread_detach(thread)) {
        print_error("pthread_detach() failed");
        return;
    }
}

void destroy_osd(void)
{
    s_stopped = true;
}

void send_osd(struct mg_connection *nc, struct http_message *hm)
{
    char *mem;
    int fd;
    struct stat sb;
    size_t png_file_size;

    fd = open(s_png_path, O_RDONLY);
    if (fd == -1) {
        print_error("open() failed");
        // TODO: server internal error
        return;
    }
    if (fstat(fd, &sb) == -1) {
        print_error("fstat() failed");
        // TODO: server internal error
        return;
    }
    png_file_size = sb.st_size;

    mem = mmap(NULL, png_file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        print_error("mmap() failed");
        // TODO: server internal error
        return;
    }

    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                  "Cache-Control: no-cache, must-revalidate\r\n"
                  "Content-Type: image/png\r\n"
                  "\r\n", (int)png_file_size);
    mg_send(nc, mem, png_file_size);

    munmap(mem, png_file_size);
    close(fd);

    print_log("send png size = %d", (int)png_file_size);
}
