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
#include "notify.h"
#include "nx_model.h"
#include "util.h"
#include "video.h"
#include "xwin.h"

#define XWIN_SEGMENT_PIXELS 320
#define XWIN_BUF_SIZE (2 + XWIN_SEGMENT_PIXELS * 4) // 2 bytes (INDEX) + 320 pixels (BGRA)
#define XWIN_NUM_SEGMENTS (get_frame_width() * get_frame_height() / XWIN_SEGMENT_PIXELS)
#define XWD_SKIP_BYTES 3179

static int s_xwin_fps;
static bool s_stopped;

static bool get_active_window_info(
    int *window_id, int *x, int *y, int *w, int *h)
{
    char command_line[256];
    FILE *command_pipe;
    char line[256];
    int line_num;

    snprintf(command_line, sizeof(command_line),
             "%s xdotool getactivewindow getwindowgeometry",
             get_chroot_command());

    command_pipe = popen(command_line, "r");
    if (command_pipe == NULL) {
        print_error("popen() failed");
        return false;
    }

    line_num = 1;
    while (fgets(line, sizeof(line), command_pipe) != NULL) {
        if (line_num == 1) {
            if (sscanf(line, "Window %d", window_id) != 1) {
                return false;
            }
        } else if (line_num == 2) {
            if (sscanf(line, "  Position: %d,%d", x, y) != 2) {
                return false;
            }
        } else if (line_num == 3) {
            if (sscanf(line, "  Geometry: %dx%d", w, h) != 2) {
                return false;
            }
        }
        line_num++;
    }

    pclose(command_pipe);

    //print_log("window_id = %d, %dx%d+%d,%d", *window_id, *w, *h, *x, *y);

    return true;
}

void init_xwin(void)
{
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
    int xwin_frame_size = get_xwin_frame_size();
    bool err = false;
    char xwd_command[64];
    int window_id, x, y, w, h;
    int xwin_num_segments = XWIN_NUM_SEGMENTS;
    bool evf_on = false;;

    free(sockets);

    snprintf(xwd_command, sizeof(xwd_command), "xwd -root");

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

        if (is_nx1()) {
            bool ret = get_active_window_info(&window_id, &x, &y, &w, &h);
            if (ret == false) {
                usleep(100*1000);
                continue;
            }
            if (w == 1024 && h == 768) {
                if (evf_on == false) {
                    evf_on = true;
                    print_log("evf on!");
                    set_video_evf(true);
                    notify_evf_start();
                }
                usleep(100*1000);
                continue;
            } else {
                evf_on = false;
                notify_evf_end();
                set_video_evf(false);
            }

            snprintf(xwd_command, sizeof(xwd_command),
                     "xwd -id %d", window_id);
            xwin_frame_size = w * h * 4;
            xwin_num_segments = w * h / XWIN_SEGMENT_PIXELS;
        }

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
                } else if (offset != xwin_frame_size &&
                           read_size != XWIN_BUF_SIZE - 2) {
                    print_log("read_size != %d (XWIN_BUF_SIZE)", XWIN_BUF_SIZE - 2);
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
                        if (is_nx1()) {
                            uint32_t *p = (uint32_t *)(&buf[2]);
                            *p = htonl(x);
                            p = (uint32_t *)(&buf[6]);
                            *p = htonl(y);
                            p = (uint32_t *)(&buf[10]);
                            *p = htonl(w);
                            p = (uint32_t *)(&buf[14]);
                            *p = htonl(h);
                        }
                        write_size = write(client_socket, buf, XWIN_BUF_SIZE);
                        if (write_size != XWIN_BUF_SIZE) {
                            print_log("write() failed");
                            err = true;
                            break;
                        }

                        if (skip_count != xwin_num_segments - 1) {
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
