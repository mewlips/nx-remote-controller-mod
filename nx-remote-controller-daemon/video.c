#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "util.h"
#include "network.h"
#include "video.h"
#include "nx_model.h"

//#define MMAP_SIZE_2 695296 // FIXME
#define DEV_MEM_PATH "/dev/mem"

static int s_video_fps;
static bool s_stopped;

static void *mmap_lcd(const int fd, const off_t offset)
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

static void munmap_lcd(void *addr, const off_t offset)
{
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    const size_t length = get_mmap_video_size();
    if (munmap(addr - (offset - pa_offset), length) == -1) {
        die("munmap() failed");
    }
}

void init_video(void)
{
    set_video_fps(get_default_video_fps());
}

void set_video_fps(int fps)
{
    s_video_fps = fps;
    print_log("video fps = %d", s_video_fps);
}

void *start_video_capture(Sockets *sockets)
{
    int client_socket = sockets->client_socket;
    int fd;
    void **addrs;
    int *hashs;
    int i, j, hash;
    long long start_time, end_time, time_diff;
    long long frame_time = 1000ll / (long)s_video_fps;
#ifdef DEBUG
    long long capture_start_time, capture_end_time;
#endif
    const int frame_width = get_frame_width();
    const int video_frame_size = get_frame_size();
    const int num_video_addrs = get_num_video_addrs();
    bool err = false;

    free(sockets);

    addrs = (void **)malloc(sizeof(void *) * num_video_addrs);
    hashs = (int *)calloc(num_video_addrs, sizeof(int));
    if (addrs == NULL || hashs == NULL) {
        die("memory allocation failed.");
    }

    fd = open(DEV_MEM_PATH, O_RDWR);
    if (fd == -1) {
        die("open() error");
    }

    for (i = 0; i < num_video_addrs; i++) {
        addrs[i] = mmap_lcd(fd, get_video_addr(i));
        print_log("phy addr = 0x%lx --> addr = 0x%p",
                  get_video_addr(i), addrs[i]);
    }

#ifdef DEBUG
    capture_start_time = get_current_time();
#endif
    s_stopped = false;
    while (!s_stopped) {
        start_time = get_current_time();

        for (i = 0; i < num_video_addrs; i++) {
            const char *p = addrs[i];

            hash = 0;
            for (j = 0; j < frame_width * 2; j++) {
                hash += p[j];
            }
            if (hashs[i] != 0 && hash != hashs[i]) {
                if (write(client_socket, p, video_frame_size) == -1) {
                    print_log("write() failed!");
                    err = true;
                    break;
                }
                //print_log("[VideoCapture] %d, hash = %d (changed!)", i, hash);
            }

            hashs[i] = hash;
        }

        end_time = get_current_time();

        time_diff = end_time - start_time;
        if (time_diff < frame_time) {
            //print_log("sleep %lld ms", frame_time - time_diff);
            usleep((frame_time - time_diff) * 1000);
        }
        frame_time = 1000ll / (long)s_video_fps;

        if (err) {
            break;
        }
    }

#ifdef DEBUG
    capture_end_time = get_current_time();
    print_log("time = %f", (capture_end_time - capture_start_time) / 1000.0);
#endif

    for (i = 0; i < num_video_addrs; i++) {
        munmap_lcd(addrs[i], get_video_addr(i));
    }

    if (close(fd) == -1) {
        print_error("close failed");
    }

    free(addrs);
    free(hashs);

    return NULL;
}

void stop_video_capture(void)
{
    s_stopped = true;
}
