#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "nx_model.h"
#include "util.h"
#include "xwin.h"

#define XWIN_SEGMENT_PIXELS 320
#define XWIN_BUF_SIZE (2 + XWIN_SEGMENT_PIXELS * 4) // 2 bytes (INDEX) + 320 pixels (BGRA)
#define XWIN_NUM_SEGMENTS (get_frame_width() * get_frame_height() / XWIN_SEGMENT_PIXELS)
#define XWD_SKIP_BYTES 3179

static int s_xwin_fps;
static bool s_stopped;
static int s_di_camera_app_id;

void init_xwin(int di_camera_app_id)
{
    s_di_camera_app_id = di_camera_app_id;
    if (s_di_camera_app_id != 0) {
        print_log("di-camera-app window-id : %d", di_camera_app_id);
    }
    set_xwin_fps(get_default_xwin_fps());
}

void set_xwin_fps(int fps)
{
    s_xwin_fps = fps;
    print_log("xwin fps = %d", s_xwin_fps);
}

void *start_xwin_capture(Sockets *sockets)
{
    int client_socket = sockets->client_socket;

    long long start_time, end_time, time_diff;
    long long frame_time = 1000ll / (long)s_xwin_fps;
#ifdef DEBUG
    long long capture_start_time, capture_end_time;
#endif
    int count, skip_count;

    FILE *xwd_out;
    unsigned char buf[XWIN_BUF_SIZE];
    size_t skip_size, read_size, offset;
    ssize_t write_size;
    int *hashs;
    int hash, hash_index;
    const int xwin_frame_size = get_xwin_frame_size();
    bool err = false;
    char xwd_command[64];

    free(sockets);

    if (s_di_camera_app_id != 0) {
        snprintf(xwd_command, sizeof(xwd_command),
                 "xwd -id %d", s_di_camera_app_id);
    } else {
        snprintf(xwd_command, sizeof(xwd_command), "xwd -root");
    }

    hashs = (int *)calloc(XWIN_NUM_SEGMENTS, sizeof(int *));

#ifdef DEBUG
    capture_start_time = get_current_time();
#endif
    count = 0;
    s_stopped = false;
    while (!s_stopped) {
        start_time = get_current_time();
        hash = 0;
        err = false;

        xwd_out = popen(xwd_command, "r");
        if (xwd_out == NULL) {
            print_error("popen() failed");
            continue;
        }

        skip_size = XWD_SKIP_BYTES;
        do {
            read_size = fread(buf, 1, skip_size < XWIN_BUF_SIZE
                                        ? skip_size : XWIN_BUF_SIZE,
                              xwd_out);
            if (read_size == 0) {
                err = true;
                print_log("xwd read_size = 0");
                break;
            }
            skip_size -= read_size;
        } while (skip_size != 0);

        if (err) {
            pclose(xwd_out);
            continue;
        }

        if (skip_size == 0) {
            offset = 0;
            hash_index = 0;
            skip_count = 0;
            while (true) {
                read_size = fread(buf + 2, 1, XWIN_BUF_SIZE - 2, xwd_out); // first 2 bytes is index
                offset += read_size;
                if (read_size == 0) {
                    print_log("read_size == 0");
                    err = true;
                    break;
                } else if (read_size != XWIN_BUF_SIZE - 2) {
                    print_log("read_size != %d (XWIN_BUF_SIZE)", XWIN_BUF_SIZE);
                    err = true;
                    break;
                } else {
                    int i;
                    for (i = 2; i < read_size + 2; i += 4) {
                        hash += buf[i];
                        if (buf[i] > 0) {
                            hash += i;
                        }
                    }

                    if (hash_index != 0 && hashs[hash_index] == hash) {
                        skip_count++;
                    } else {
                        hashs[hash_index] = hash;
                        buf[0] = (hash_index >> 8) & 0xff;
                        buf[1] = hash_index & 0xff;

                        write_size = write(client_socket, buf, XWIN_BUF_SIZE);
                        if (write_size != XWIN_BUF_SIZE) {
                            print_log("write() failed");
                            err = true;
                            break;
                        }
                    }
                    if (offset == xwin_frame_size) {
                        // notify end of frame
                        buf[0] = 0x0f;
                        buf[1] = 0xff;
                        write_size = write(client_socket, buf, XWIN_BUF_SIZE);
                        if (write_size != XWIN_BUF_SIZE) {
                            print_log("write() failed");
                            err = true;
                            break;
                        }

                        if (skip_count != XWIN_NUM_SEGMENTS - 1) {
                            print_log("[XWinCapture] count = %d, skip_count = %d",
                                count, skip_count);
                        }
                        break;
                    }
                }
                hash_index++;
            }
        } else {
            print_log("skip_size = %d", skip_size);
            err = true;
        }

        if (pclose(xwd_out) == -1) {
            //print_error("pclose() failed");
            //err = true;
        }

        end_time = get_current_time();

        time_diff = end_time - start_time;
        if (time_diff < frame_time) {
            //print_log("sleep %lld ms", frame_time - time_diff);
            usleep((frame_time - time_diff) * 1000);
        }
        count++;
        frame_time = 1000ll / (long)s_xwin_fps;

        if (err) {
            break;
        }
    }
#ifdef DEBUG
    capture_end_time = get_current_time();
    print_log("time = %f", (capture_end_time - capture_start_time) / 1000.0);
#endif

    free(hashs);

    return NULL;
}

void stop_xwin_capture(void)
{
    s_stopped = true;
}
