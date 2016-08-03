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

#define DEV_MEM_PATH "/dev/mem"

static int s_frame_width;
static int s_frame_height;
static int s_num_video_addrs;
static void **s_addrs;
static int *s_hashs;
static int s_fd;

void *mmap_lcd(const int fd, const off_t offset)
{
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    //print_log("offset = %llu, pa_offset = %llu", (unsigned long long)offset, (unsigned long long)pa_offset);
    const size_t length = get_mmap_video_size();
    void *p = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, pa_offset);
    if (p == MAP_FAILED) {
        die("mmap() failed");
    }

    return p + (offset - pa_offset);
}

void munmap_lcd(void *addr, const off_t offset)
{
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    const size_t length = get_mmap_video_size();
    if (munmap(addr - (offset - pa_offset), length) == -1) {
        die("munmap() failed");
    }
}

void liveview_init(void)
{
    int i;

    s_frame_width = get_frame_width();
    s_frame_height = get_frame_height();
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

void liveview_destroy(void)
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

void liveview_send(struct mg_connection *nc, struct http_message *hm)
{
    int i, j, hash;
    int x, y;
    static unsigned char *nv12_mem = NULL;
    unsigned char ys[800*480/4];
    int frame_size = get_frame_size();;
    bool reduce_size = true; // TODO

    if (reduce_size) {
        frame_size = s_frame_width * s_frame_height / 4 * 3;
    }

    for (i = 0; i < s_num_video_addrs; i++) {
        unsigned char *p = s_addrs[i];

        hash = 0;
        for (j = 0; j < s_frame_width * 2; j++) {
            hash += p[j];
        }
        if (s_hashs[i] != 0 && hash != s_hashs[i]) {
            nv12_mem = p;
            print_log("[%d] framesize = %d", i, frame_size);
        }

        s_hashs[i] = hash;
    }

    if (nv12_mem != NULL) {
        if (reduce_size) {
            i = 0;
            for (y = 0; y < s_frame_height; y+=2) {
                for (x = 0; x < s_frame_width; x+=2) {
                    ys[i++] = nv12_mem[y * s_frame_width + x];
                }
            }
        }

        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                      "Content-Type: text/plain; charset=x-user-defined\r\n"
                      "\r\n", frame_size);
        if (reduce_size) {
            mg_send(nc, ys, s_frame_width * s_frame_height / 4);
            mg_send(nc, nv12_mem + (s_frame_width * s_frame_height),
                    s_frame_width * s_frame_height / 4 * 2);
        } else {
            mg_send(nc, nv12_mem, frame_size);
        }
    } else {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                      "Content-Type: text/plain; charset=x-user-defined\r\n"
                      "\r\n", 0);
    }
}
