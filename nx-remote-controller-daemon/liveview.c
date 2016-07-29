#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mongoose.h"
#include "nx_model.h"
#include "util.h"
#include "video.h"

#define DEV_MEM_PATH "/dev/mem"

static int s_frame_width;
static int s_video_frame_size;
static int s_num_video_addrs;
static void **s_addrs;
static int *s_hashs;
static int s_fd;

void init_liveview(void)
{
    int i;

    set_video_fps(get_default_video_fps());
    s_frame_width = get_frame_width();
    s_video_frame_size = get_frame_size();
    s_num_video_addrs = get_num_video_addrs();

    s_addrs = (void **)malloc(sizeof(void *) * s_num_video_addrs);
    s_hashs = (int *)calloc(s_num_video_addrs, sizeof(int));

    if (s_addrs == NULL || s_hashs == NULL) {
        die("memory allocation failed.");
    }

    s_fd = open(DEV_MEM_PATH, O_RDWR);
    if (s_fd == -1) {
        die("open() error");
    }

    for (i = 0; i < s_num_video_addrs; i++) {
        s_addrs[i] = mmap_lcd(s_fd, get_video_addr(i));
        print_log("phy addr = 0x%lx --> addr = 0x%p",
                  get_video_addr(i), s_addrs[i]);
    }
}

void destroy_liveview(void)
{
    int i;

    for (i = 0; i < s_num_video_addrs; i++) {
        munmap_lcd(s_addrs[i], get_video_addr(i));
    }

    if (close(s_fd) == -1) {
        print_error("close failed");
    }

    free(s_addrs);
    free(s_hashs);
}

void send_liveview(struct mg_connection *nc, struct http_message *hm)
{
    int i, j, hash;
    const char *nv12_mem = NULL;

    for (i = 0; i < s_num_video_addrs; i++) {
        const char *p = s_addrs[i];

        hash = 0;
        for (j = 0; j < s_frame_width * 2; j++) {
            hash += p[j];
        }
        if (s_hashs[i] != 0 && hash != s_hashs[i]) {
            nv12_mem = p;
            print_log("[%d] framesize = %d", i, s_video_frame_size);
        }

        s_hashs[i] = hash;
    }

    if (nv12_mem != NULL) {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                      "Content-Type: text/plain; charset=x-user-defined\r\n"
                      "\r\n", s_video_frame_size);
        mg_send(nc, nv12_mem, s_video_frame_size);
    } else {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                      "Content-Type: text/plain; charset=x-user-defined\r\n"
                      "\r\n", 0);
    }
}
