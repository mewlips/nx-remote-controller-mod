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
#include "lcd.h"
#include "nx_model.h"
#include "util.h"
#include "mongoose.h"

#define NX1_500_FRAME_WIDTH  720
#define NX300_FRAME_WIDTH    800
#define FRAME_HEIGHT         480
#define BLOCK_WIDTH           80
#define BLOCK_HEIGHT          40
#define NUM_BLOCKS (NX300_FRAME_WIDTH * FRAME_HEIGHT / BLOCK_WIDTH / BLOCK_HEIGHT)
#define XWD_SKIP_BYTES 3179

static bool s_stopped;
static unsigned char s_buffer[NX300_FRAME_WIDTH * FRAME_HEIGHT * 4 + 6]; // 6 -> sometimes, XWD_SKIP_BYTES is 3185
static long long s_request_time;
static int s_di_camera_app_window_id;
static bool s_evf;
static bool s_buffer_shifted_by_6;

static bool get_active_window_info(int *window_id,
                                   int *x, int *y, int *w, int *h)
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

static int get_di_camera_app_window_id(void)
{
    char command[256];
    FILE *command_pipe;
    char buf[32];
    int window_id = -1;

    snprintf(command, sizeof(command),
             "%s xdotool search --class di-camera-app", get_chroot_command());
    command_pipe = popen(command, "r");
    if (fgets(buf, sizeof(buf), command_pipe) != NULL) {
        window_id = atoi(buf);
    }
    pclose(command_pipe);

    return window_id;
}

static void *cap_osd_thread(void *data)
{
    char command[256];
    FILE *command_pipe;
    bool need_xwd_skip_bytes;
    size_t read_size, offset;
    int window_id, x, y, w, h;
    int fd;
    int frame_width, frame_height;
    long long start_time;//, end_time;

    while (!s_stopped) {
        need_xwd_skip_bytes = false;
        start_time = get_current_time();
        if (s_request_time < start_time - 1000) {
            usleep(50*1000);
            continue;
        }

        if (get_active_window_info(&window_id, &x, &y, &w, &h) == false) {
            print_error("get_active_window_info() failed");
            continue;
        }

        if (is_nx1() || is_nx500()) {
            frame_width  = NX1_500_FRAME_WIDTH;
        } else { // NX300
            frame_width  = NX300_FRAME_WIDTH;
        }
        frame_height = FRAME_HEIGHT;

        if (is_nx1()) {
            if (w == 1024 && h == 768 && x == 720) { // NX1 EVF
                s_evf = true;
                snprintf(command, sizeof(command),
                         "xwd -id %d | "
                         "%s convert -size 1024x768+%d -depth 8 bgra:- "
                         "-scale x480 -background \"rgba(0,0,0,255)\" "
                         "-gravity center -extent 720x480 bgra:-",
                         window_id, get_chroot_command(), XWD_SKIP_BYTES);
            } else if (w == 720 && h == 480) {
                s_evf = false;
                snprintf(command, sizeof(command),
                         "xwd -id %d", window_id);
                need_xwd_skip_bytes = true;
            } else {
                s_evf = false;
                snprintf(command, sizeof(command),
                         "xwd -root | "
                         "%s convert -size 1744x768+%d -depth 8 bgra:- "
                         "-crop 720x480+0+0 bgra:-",
                         get_chroot_command(), XWD_SKIP_BYTES);
            }
        } else if (is_nx500()) {
            if (w == 720 && h == 480 && lcd_get_state() == LCD_OFF) {
                snprintf(command, sizeof(command), "xwd -id %d",
                         s_di_camera_app_window_id);
            } else {
                snprintf(command, sizeof(command), "xwd -root");
            }
            need_xwd_skip_bytes = true;
        } else { // NX300
            if (w == 480 && h == 800) {
                snprintf(command, sizeof(command),
                         "xwd -id %d | "
                         "%s convert -size 480x800+%d -depth 8 bgra:- "
                         "-rotate 90 bgra:-",
                         window_id, get_chroot_command(), XWD_SKIP_BYTES);
            } else {
                snprintf(command, sizeof(command),
                         "xwd -root | "
                         "%s convert -size 480x800+%d -depth 8 bgra:- "
                         "-rotate 90 bgra:-",
                         get_chroot_command(), XWD_SKIP_BYTES);
            }
        }

        //print_log("command = %s", command);
        command_pipe = popen(command, "r");
        if (command_pipe == NULL) {
            print_error("popen() failed");
            usleep(50*1000);
            continue;;
        }
        fd = fileno(command_pipe);

        if (need_xwd_skip_bytes) {
            read_size = 0;
            offset = 0;
            while (read_size < XWD_SKIP_BYTES) {
                read_size = read(fd, s_buffer + offset, XWD_SKIP_BYTES - read_size);
                if (read_size == 0) {
                    print_log("EOF");
                    break;
                } else if (read_size == -1) {
                    print_error("read() failed");
                    break;
                }
                offset += read_size;
            }
            if (offset != XWD_SKIP_BYTES) {
                print_log("read_size not match! %d", offset);
                pclose(command_pipe);
                continue;
            }
        }

        read_size = 0;
        offset = 0;
        while (offset < frame_width * frame_height * 4) {
            read_size = read(fd, s_buffer + offset, sizeof(s_buffer) - offset);
            if (read_size == -1) {
                print_error("read() failed");
                break;
            }
            offset += read_size;
        }
        if (offset == frame_width * frame_height * 4 + 6) {
            s_buffer_shifted_by_6 = true;
        } else if (offset != frame_width * frame_height * 4) {
            print_log("frame read failed! offset = %d", offset);
        } else {
            s_buffer_shifted_by_6 = false;
        }

        pclose(command_pipe);

        //end_time = get_current_time();
        //print_log("cap_osd_thread() time = %lld", end_time - start_time);
    }

    print_log("cap_osd_thread() terimanted.");

    return NULL;
}

void osd_init(void)
{
    pthread_t thread;

    s_stopped = false;

    s_di_camera_app_window_id = get_di_camera_app_window_id();
    print_log("di-camera-app = %d", s_di_camera_app_window_id);

    if (pthread_create(&thread, NULL, cap_osd_thread, NULL)) {
        print_error("pthread_create() failed");
        return;
    }
    if (pthread_detach(thread)) {
        print_error("pthread_detach() failed");
        return;
    }
}

void osd_destroy(void)
{
    s_stopped = true;
}

struct Block {
    uint16_t x_offset;
    uint16_t y_offset;
    uint32_t hash;
    uint8_t pixels[BLOCK_WIDTH * BLOCK_HEIGHT * 4]; // RGBA pixels
};

struct OsdData {
    uint16_t frame_width;
    uint16_t frame_height;
    uint16_t block_width;
    uint16_t block_height;
    struct Block blocks[NUM_BLOCKS];
};

static void osd_get(int *last_hashs, struct OsdData *data, int *send_count)
{
    int x, y;
    int c, i, j;
    uint16_t x_offset, y_offset;
    int block_index;

    if (is_nx1() || is_nx500()) {
        data->frame_width  = NX1_500_FRAME_WIDTH;
    } else { // NX300
        data->frame_width  = NX300_FRAME_WIDTH;
    }
    data->frame_height = FRAME_HEIGHT;
    data->block_width  = BLOCK_WIDTH;
    data->block_height = BLOCK_HEIGHT;

    //print_log("command = %s", command);

    if (s_buffer_shifted_by_6) {
        i = 6;
    } else {
        i = 0;
    }

    c = 0;
    *send_count = 0;
    for (y_offset = 0; y_offset < data->frame_height; y_offset++) {
        for (x_offset = 0; x_offset < data->frame_width; x_offset++) {
            block_index = ((y_offset / data->block_height) *
                           (data->frame_width / data->block_width)) +
                          (x_offset / data->block_width);
            if ((x_offset % data->block_width) == 0 &&
                    (y_offset % data->block_height) == 0) {
                data->blocks[block_index].x_offset = x_offset;
                data->blocks[block_index].y_offset = y_offset;
                data->blocks[block_index].hash = 0;
                //print_log("block_index = %d, x = %d, y = %d",
                //          block_index, x_offset, y_offset);
            }
            x = x_offset % data->block_width;
            y = y_offset % data->block_height;
            for (j = 2; j >= 0; j--) { // BGRA --> RGBA
                c = s_buffer[i++];
                data->blocks[block_index]
                    .pixels[(y * data->block_width * 4) + (x * 4) + j] = c;
                data->blocks[block_index].hash += c + x_offset + y_offset;
            }
            c = s_buffer[i++];
            data->blocks[block_index]
                .pixels[(y * data->block_width * 4) + (x * 4) + 3] = c;
            data->blocks[block_index].hash += c + x_offset + y_offset;
            if (x == data->block_width - 1 && y == data->block_height - 1) {
                //print_log("last = %d, hash = %d",
                //          last_hashs[block_index],
                //          data->blocks[block_index].hash);
                if (last_hashs[block_index] != data->blocks[block_index].hash) {
                    (*send_count)++;
                }
            }
        }
    }
}

void osd_http_send(struct mg_connection *nc, struct http_message *hm)
{
    int last_hashs[NUM_BLOCKS];
    int i;
    char hex[9];
    struct OsdData data;
    int send_count;
    int num_blocks;
    long long end_time;
    uint32_t x = 0xffffffff; // end of data

    s_request_time = get_current_time();

    hex[8] = '\0';
    for (i = 0; i < (hm->body.len - 6) / 8; i++) {
        strncpy(hex, (hm->body.p + 6) + i * 8, 8);
        last_hashs[i] = (uint32_t)strtol(hex, NULL, 16);
        //print_log("hex = %s", hex);
    }

    osd_get(last_hashs, &data, &send_count);

    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                  "Content-Type: text/plain; charset=x-user-defined\r\n"
                  "\r\n", 8 + send_count * sizeof(data.blocks[0]) + 4);

    num_blocks = data.frame_width * data.frame_height
                 / data.block_width / data.block_height;

    data.frame_width = htons(data.frame_width);
    data.frame_height = htons(data.frame_height);
    data.block_width = htons(data.block_width);
    data.block_height = htons(data.block_height);
    mg_send(nc, (const void *)&data, 8);

    send_count = 0;
    for (i = 0; i < num_blocks; i++) {
        if (data.blocks[i].hash != last_hashs[i]) {
            data.blocks[i].x_offset = htons(data.blocks[i].x_offset);
            data.blocks[i].y_offset = htons(data.blocks[i].y_offset);
            data.blocks[i].hash = htonl(data.blocks[i].hash);
            mg_send(nc, &data.blocks[i], sizeof(data.blocks[i]));
            send_count++;
        }
    }

    mg_send(nc, &x, 4);

    end_time = get_current_time();
    print_log("send_count = %d, time = %lld",
              send_count, end_time - s_request_time);
}

bool osd_is_evf(void)
{
    return s_evf;
}
