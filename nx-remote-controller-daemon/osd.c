#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "api_server.h"
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

static bool s_stopped;
static unsigned char s_buffer[NX300_FRAME_WIDTH * FRAME_HEIGHT * 4 + 4096]; // 4096 for header
static long long s_request_time;
static bool s_evf;
static int s_buffer_shift_bytes;
static struct OsdData s_osd_data;
static bool s_osd_data_ready;
static pthread_mutex_t s_mutex;


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

static void update_osd_data(void)
{
    int x, y;
    int c, i, j;
    uint16_t x_offset, y_offset;
    int block_index;
    int num_blocks;

    pthread_mutex_lock(&s_mutex);

    if (is_nx1() || is_nx500()) {
        s_osd_data.frame_width  = NX1_500_FRAME_WIDTH;
    } else { // NX300
        s_osd_data.frame_width  = NX300_FRAME_WIDTH;
    }
    s_osd_data.frame_height = FRAME_HEIGHT;
    s_osd_data.block_width  = BLOCK_WIDTH;
    s_osd_data.block_height = BLOCK_HEIGHT;

    if (s_buffer_shift_bytes > 0) {
        i = s_buffer_shift_bytes;
    } else {
        i = 0;
    }

    c = 0;
    for (y_offset = 0; y_offset < s_osd_data.frame_height; y_offset++) {
        for (x_offset = 0; x_offset < s_osd_data.frame_width; x_offset++) {
            block_index = ((y_offset / s_osd_data.block_height) *
                           (s_osd_data.frame_width / s_osd_data.block_width)) +
                          (x_offset / s_osd_data.block_width);
            if ((x_offset % s_osd_data.block_width) == 0 &&
                    (y_offset % s_osd_data.block_height) == 0) {
                s_osd_data.blocks[block_index].x_offset = x_offset;
                s_osd_data.blocks[block_index].y_offset = y_offset;
                s_osd_data.blocks[block_index].hash = 0;
                //print_log("block_index = %d, x = %d, y = %d",
                //          block_index, x_offset, y_offset);
            }
            x = x_offset % s_osd_data.block_width;
            y = y_offset % s_osd_data.block_height;
            for (j = 2; j >= 0; j--) { // BGRA --> RGBA
                c = s_buffer[i++];
                s_osd_data.blocks[block_index]
                    .pixels[(y * s_osd_data.block_width * 4) + (x * 4) + j] = c;
                s_osd_data.blocks[block_index].hash += c + x_offset + y_offset;
            }
            c = s_buffer[i++];
            s_osd_data.blocks[block_index]
                .pixels[(y * s_osd_data.block_width * 4) + (x * 4) + 3] = c;
            s_osd_data.blocks[block_index].hash += c + x_offset + y_offset;
        }
    }

    num_blocks = s_osd_data.frame_width * s_osd_data.frame_height
                 / s_osd_data.block_width / s_osd_data.block_height;
    for (i = 0; i < num_blocks; i++) {
        s_osd_data.blocks[i].x_offset = htons(s_osd_data.blocks[i].x_offset);
        s_osd_data.blocks[i].y_offset = htons(s_osd_data.blocks[i].y_offset);
        s_osd_data.blocks[i].hash = htonl(s_osd_data.blocks[i].hash);
    }
    s_osd_data.frame_width = htons(s_osd_data.frame_width);
    s_osd_data.frame_height = htons(s_osd_data.frame_height);
    s_osd_data.block_width = htons(s_osd_data.block_width);
    s_osd_data.block_height = htons(s_osd_data.block_height);

    pthread_mutex_unlock(&s_mutex);

    s_osd_data_ready = true;
}


static void *cap_osd_thread(void *data)
{
    char command[256];
    FILE *command_pipe;
    size_t read_size, offset;
    int window_id, x, y, w, h;
    int fd;
    int frame_width, frame_height;
    long long start_time, mid_time, end_time;

    while (!s_stopped) {
        start_time = get_current_time();
        if (s_request_time < start_time - 2000) {
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
                snprintf(command, sizeof(command), "xwd -id %lu",
                         get_di_camera_app_window_id());
            } else {
                snprintf(command, sizeof(command), "xwd -root");
            }
        } else { // NX300
            if (w == 480 && h == 800) {
                snprintf(command, sizeof(command),
                         "xwd -id %d | "
                         "%s convert -size 480x800+%d -depth 8 bgra:- "
                         "-rotate 270 bgra:-",
                         window_id, get_chroot_command(), XWD_SKIP_BYTES);
            } else {
                snprintf(command, sizeof(command),
                         "xwd -root | "
                         "%s convert -size 480x800+%d -depth 8 bgra:- "
                         "-rotate 270 bgra:-",
                         get_chroot_command(), XWD_SKIP_BYTES);
            }
        }

        //print_log("command = %s", command);
        command_pipe = popen(command, "r");
        if (command_pipe == NULL) {
            print_error("popen() failed");
            usleep(50*1000);
            continue;
        }
        fd = fileno(command_pipe);

        read_size = 0;
        offset = 0;
        while (offset < sizeof(s_buffer)) {
            read_size = read(fd, s_buffer + offset, sizeof(s_buffer) - offset);
            if (read_size == -1) {
                print_error("read() failed");
                break;
            } else if (read_size == 0) {
                //print_log("read_size is 0");
                break;
            }
            offset += read_size;
        }
        if (offset > frame_width * frame_height * 4) {
            s_buffer_shift_bytes = offset - (frame_width * frame_height * 4);
            //print_log("shift bytes = %d", s_buffer_shift_bytes);
        } else if (offset < frame_width * frame_height * 4) {
            print_log("frame read failed! offset = %d", offset);
            pclose(command_pipe);
            continue;
        } else {
            s_buffer_shift_bytes = 0;
        }

        pclose(command_pipe);

        mid_time = get_current_time();

        update_osd_data(); // TODO: mutex lock?
        end_time = get_current_time();
        print_log("cap_osd_thread() time = %lld + %lld = %lld",
                mid_time - start_time,
                end_time - mid_time,
                end_time - start_time);
        //usleep(50*1000);
    }

    print_log("cap_osd_thread() terimanted.");

    return NULL;
}

void osd_init(void)
{
    pthread_t thread;

    pthread_mutex_init(&s_mutex, NULL);

    s_stopped = false;

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

    pthread_mutex_destroy(&s_mutex);
}


void osd_http_send(struct mg_connection *nc, struct http_message *hm)
{
    int last_hashs[NUM_BLOCKS];
    int i;
    char hex[9];
    int send_count;
    int num_blocks;
    long long end_time;
    uint32_t x = 0xffffffff; // end of data

    s_request_time = get_current_time();

    if (s_osd_data_ready == false) {
        //print_log("s_osd_data not ready");
        send_200(nc);
        return;
    }

    hex[8] = '\0';
    for (i = 0; i < (hm->body.len - 6) / 8; i++) {
        strncpy(hex, (hm->body.p + 6) + i * 8, 8);
        last_hashs[i] = (uint32_t)strtol(hex, NULL, 16);
        //print_log("hex = %s", hex);
    }

    pthread_mutex_lock(&s_mutex);

    num_blocks = ntohs(s_osd_data.frame_width) * ntohs(s_osd_data.frame_height)
                 / ntohs(s_osd_data.block_width) / ntohs(s_osd_data.block_height);
    //print_log("num_blocks = %d", num_blocks);

    send_count = 0;
    for (i = 0; i < num_blocks; i++) {
        if (ntohl(s_osd_data.blocks[i].hash) != last_hashs[i]) {
            send_count++;
        }
    }

    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                  "Content-Type: text/plain; charset=x-user-defined\r\n"
                  "\r\n", 8 + send_count * sizeof(s_osd_data.blocks[0]) + 4);

    mg_send(nc, (const void *)&s_osd_data, 8);

    send_count = 0;
    for (i = 0; i < num_blocks; i++) {
        if (ntohl(s_osd_data.blocks[i].hash) != last_hashs[i]) {
            mg_send(nc, &s_osd_data.blocks[i], sizeof(s_osd_data.blocks[i]));
            send_count++;
        }
    }

    pthread_mutex_unlock(&s_mutex);

    mg_send(nc, &x, 4);

    end_time = get_current_time();
    if (send_count != 0) {
        print_log("send_count = %d, time = %lld",
                  send_count, end_time - s_request_time);
    }
}

bool osd_is_evf(void)
{
    return s_evf;
}
