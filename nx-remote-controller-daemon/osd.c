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
static unsigned char s_pngs[1024*1024][2];
static size_t s_pngs_size[2];
static int s_png_current_index;
static long long s_request_time;

static void *cap_osd_thread(void *data)
{
    char osd_cap_command[128];
    FILE *command_pipe;
    int c, i;
    unsigned char *p;

    snprintf(osd_cap_command, sizeof(osd_cap_command), "%s osd_cap2.sh %s scale", // TODO: scale / sample / ""
             get_chroot_command(), get_nx_model_name());

    //print_log("osd_cap_command = %s", osd_cap_command);

    while (!s_stopped) {
        if (s_request_time < get_current_time() - 2000) {
            usleep(50*1000);
            continue;
        }
        command_pipe = popen(osd_cap_command, "r");
        if (command_pipe == NULL) {
            print_error("popen() failed");
            // TODO: server internal error
            return NULL;
        }

        p = s_pngs[s_png_current_index];
        i = 0;
        while ((c = fgetc(command_pipe)) != EOF) {
            p[i++] = c;
        }
        s_pngs_size[s_png_current_index] = i;
        print_log("[%d] png_size = %d", s_png_current_index, s_pngs_size[s_png_current_index]);
        pclose(command_pipe);

        if (s_png_current_index == 0) {
            s_png_current_index = 1;
        } else {
            s_png_current_index = 0;
        }
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
    int index = !s_png_current_index;
    unsigned char *png_data = s_pngs[index];
    int png_size = s_pngs_size[index];

    s_request_time = get_current_time();

    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                  "Cache-Control: no-cache, must-revalidate\r\n"
                  "Content-Type: image/png\r\n"
                  "\r\n", (int)png_size);
    mg_send(nc, png_data, png_size);

    print_log("send [%d] png size = %d", index, png_size);
}
