#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef DEBUG
#define log(fmt, ...) \
    do { \
        fprintf(stderr, "[%s():%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while(0)
#else
#define log(fmt, ...)
#endif

static void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void print_error(const char *msg)
{
    perror(msg);
}

#define FRAME_WIDTH 720
#define FRAME_HEIGHT 480

#define VIDEO_FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT * 3 / 2)

#define MMAP_SIZE 522496
#define MMAP_SIZE_2 695296

#define XWIN_SEGMENT_PIXELS 320
#define XWIN_BUF_SIZE (2 + XWIN_SEGMENT_PIXELS * 4) // 2 bytes (INDEX) + 320 pixels (BGRA)
#define XWIN_NUM_SEGMENTS (FRAME_WIDTH * FRAME_HEIGHT / XWIN_SEGMENT_PIXELS)
#define XWIN_FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT * 4)

#define PORT_VIDEO 5678
#define PORT_XWIN 5679
#define PORT_EXECUTOR 5680
#define PORT_UDP_BROADCAST 5681

#define XWD_SKIP_BYTES 3179
#define DISCOVERY_PACKET_SIZE 32

static off_t s_addrs[] = {
    0xbbaea500,
    0xbbb68e00,
    0xbbbe7700,
    0xbba6bc00,

//    0xa4a0d000, // MMAP_SIZE_2
//    0xa4ad2000, // MMAP_SIZE_2
//    0xaefd0000, // MMAP_SIZE_2
//    0xaf150000, // MMAP_SIZE_2

//    0x9f600000,
//    0x9f6fd000,
//    0x9f77b000,
//    0x9f8f7000,
};

#define S_ADDRS_SIZE sizeof(s_addrs)

typedef struct {
    int server_fd;
    int client_fd;
    int fps;
} StreamerData;

typedef void *(*OnConnect)(StreamerData *data);

typedef struct {
    int port;
    OnConnect on_connect;
    int *socket_connect_count;
} ListenSocketData;

static void *mmap_lcd(const int fd, const off_t offset)
{
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    //log("offset = %llu, pa_offset = %llu", (unsigned long long)offset, (unsigned long long)pa_offset);
    void *p = mmap(NULL, MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, pa_offset);
    if (p == MAP_FAILED) {
        die("mmap() failed");
    }

    return p + (offset - pa_offset);
}

static void munmap_lcd(void *addr, const off_t offset)
{
    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    if (munmap(addr - (offset - pa_offset), MMAP_SIZE) == -1) {
        die("munmap() failed");
    }
}

static long long get_current_time()
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

static void *start_video_capture(StreamerData *data)
{
    int client_fd = data->client_fd;
    int fps = data->fps;
    int fd;
    void *addrs[S_ADDRS_SIZE];
    int hashs[S_ADDRS_SIZE] = {0,};
    int count;
    int i, j, hash;
    long long start_time, end_time, time_diff;
    long long frame_time = 1000ll / (long)fps;
#ifdef DEBUG
    long long capture_start_time, capture_end_time;
#endif
    bool err = false;

    free(data);

    fd = open("/dev/mem", O_RDWR);
    if (fd == -1) {
        die("open() error");
    }

    for (i = 0; i < S_ADDRS_SIZE; i++) {
        addrs[i] = mmap_lcd(fd, s_addrs[i]);
    }

#ifdef DEBUG
    capture_start_time = get_current_time();
#endif
    count = 0;
    while (true) {
        start_time = get_current_time();

        for (i = 0; i < S_ADDRS_SIZE; i++) {
            const char *p = addrs[i];

            hash = 0;
            for (j = 0; j < 720*2; j++) {
                hash += p[j];
            }
            if (hashs[i] != 0 && hash != hashs[i]) {
                if (write(client_fd, p, VIDEO_FRAME_SIZE) == -1) {
                    log("write() failed!");
                    err = true;
                    break;
                }
                log("[VideoCapture] count = %d (%d), hash = %d (changed!)", count, i, hash);

                count++;
            } else {
                //log("count = %d, hash = %d", count, hash);
            }

            hashs[i] = hash;
        }

        end_time = get_current_time();

        time_diff = end_time - start_time;
        if (time_diff < frame_time) {
            //log("sleep %lld ms", frame_time - time_diff);
            usleep((frame_time - time_diff) * 1000);
        }
        count++;

        if (err) {
            break;
        }
    }

#ifdef DEBUG
    capture_end_time = get_current_time();
    log("time = %f", (capture_end_time - capture_start_time) / 1000.0);
#endif

    for (i = 0; i < 4; i++) {
        munmap_lcd(addrs[i], s_addrs[i]);
    }

    if (close(fd) == -1) {
        print_error("close failed");
    }

    return NULL;
}

static void *start_xwin_capture(StreamerData *data)
{
    int client_fd = data->client_fd;
    int fps = data->fps;

    long long start_time, end_time, time_diff;
    long long frame_time = 1000ll / (long)fps;
#ifdef DEBUG
    long long capture_start_time, capture_end_time;
#endif
    int count, skip_count;

    FILE *xwd_out;
    unsigned char buf[XWIN_BUF_SIZE];
    size_t skip_size, read_size, offset;
    ssize_t write_size;
    int hashs[XWIN_NUM_SEGMENTS] = {0,};
    int hash, hash_index;;

    bool err = false;

    free(data);

#ifdef DEBUG
    capture_start_time = get_current_time();
#endif
    count = 0;
    while (true) {
        start_time = get_current_time();
        hash = 0;

        xwd_out = popen("xwd -root", "r");
        if (xwd_out == NULL) {
            print_error("popen() failed");
            break;
        }

        skip_size = XWD_SKIP_BYTES;
        do {
            read_size = fread(buf, 1, skip_size < XWIN_BUF_SIZE
                                        ? skip_size : XWIN_BUF_SIZE,
                              xwd_out);
            if (read_size == 0) {
                err = true;
                break;
            }
            skip_size -= read_size;
        } while (skip_size != 0);

        if (skip_size == 0) {
            offset = 0;
            hash_index = 0;
            skip_count = 0;
            while (true) {
                read_size = fread(buf + 2, 1, XWIN_BUF_SIZE - 2, xwd_out); // first 2 bytes is index
                offset += read_size;
                if (read_size == 0) {
                    log("read_size == 0");
                    err = true;
                    break;
                } else if (read_size != XWIN_BUF_SIZE - 2) {
                    log("read_size != %d (XWIN_BUF_SIZE)", XWIN_BUF_SIZE);
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

                    if (hashs[hash_index] == hash) {
                        skip_count++;
                    } else {
                        hashs[hash_index] = hash;
                        buf[0] = (hash_index >> 8) & 0xff;
                        buf[1] = hash_index & 0xff;

                        write_size = write(client_fd, buf, XWIN_BUF_SIZE);
                        if (write_size != XWIN_BUF_SIZE) {
                            log("write() failed");
                            err = true;
                            break;
                        }
                    }
                    if (offset == XWIN_FRAME_SIZE) {
                        // notify end of frame
                        buf[0] = 0x0f;
                        buf[1] = 0xff;
                        write_size = write(client_fd, buf, XWIN_BUF_SIZE);
                        if (write_size != XWIN_BUF_SIZE) {
                            log("write() failed");
                            err = true;
                            break;
                        }

                        if (skip_count != 1080) {
                            log("[XWinCapture] count = %d, skip_count = %d",
                                count, skip_count);
                        }
                        break;
                    }
                }
                hash_index++;
            }
        } else {
            log("skip_size = %d", skip_size);
            err = true;
        }

        if (pclose(xwd_out) == -1) {
            print_error("pclose() failed");
            err = true;
        }

        end_time = get_current_time();

        time_diff = end_time - start_time;
        if (time_diff < frame_time) {
            //log("sleep %lld ms", frame_time - time_diff);
            usleep((frame_time - time_diff) * 1000);
        }
        count++;

        if (err) {
            break;
        }
    }
#ifdef DEBUG
    capture_end_time = get_current_time();
    log("time = %f", (capture_end_time - capture_start_time) / 1000.0);
#endif

    return NULL;
}

static void *start_executor(StreamerData *data)
{
    FILE *client_sock;
    int client_fd = data->client_fd;;
    char command_line[256];
    bool err = false;
    FILE *out = NULL;
    char buf[1024];
    size_t read_size;
    size_t write_size;
    unsigned long size;

    client_sock = fdopen(data->client_fd, "r");
    if (client_sock == NULL) {
        print_error("fdopen() failed");
        err = true;
    }

    free(data);

    if (err) {
        goto error;
    }

    log("executor started.");

    while (fgets(command_line, sizeof(command_line), client_sock)) {
        command_line[strlen(command_line) - 1] = '\0'; // strip '\n' at end

        if (strlen(command_line) > 0 && command_line[0] == '@') {
            printf("%s\n", command_line + 1);
            fflush(stdout);
        } else if (strlen(command_line) > 0) {
            log("command = %s", command_line);

            out = popen(command_line, "r");
            if (out == NULL) {
                print_error("popen() failed");
                continue;
            }

            while (feof(out) == 0 && ferror(out) == 0) {
                read_size = fread(buf, 1, sizeof(buf), out);
                if (read_size == 0) {
                    break;
                }
                while (read_size > 0) {
                    size = htonl(read_size);
                    write_size = write(client_fd, (const void *)&size, 4);
                    if (write_size == -1) {
                        print_error("write() failed!");
                        goto error;
                    }
                    write_size = write(client_fd, buf, read_size);
                    if (write_size == -1) {
                        print_error("write() failed!");
                        goto error;
                    }
                    read_size -= write_size;
                }
            }
        }

        // EOF
        size = 0;
        write_size = write(client_fd, (const void *)&size, 4);
        if (write_size == -1) {
            print_error("write() failed!");
            goto error;
        }

        if (out != NULL && pclose(out) == -1) {
            print_error("pclose() failed!");
        }
        out = NULL;
    }

error:
    if (out != NULL) {
        if (pclose(out) == -1) {
            print_error("pclose() failed!");
        }
    }
    log("executor finished.");
    return NULL;
}

static void *listen_socket_func(void *thread_data)
{
    ListenSocketData *listen_socket_data = (ListenSocketData *)thread_data;
    int port = listen_socket_data->port;
    OnConnect on_connect = listen_socket_data->on_connect;
    int *socket_connect_count = listen_socket_data->socket_connect_count;
    struct sockaddr_in server_addr, client_addr;
    int server_fd, client_fd;
    socklen_t len;
    pthread_t thread;
    int on = 1;

    free(listen_socket_data);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        die("socket() failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&on, (socklen_t)sizeof(on));

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        die("bind() failed");
    }

    if (listen(server_fd, 5) == -1) {
        die("listen() failed");
    }

    len = sizeof(client_addr);

    while (true) {
        StreamerData *data = (StreamerData *)malloc(sizeof(StreamerData));

        log("waiting client... port = %d", port);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd == -1) {
            die("accept() failed");
        }

        log("client connected. port = %d", port);

        data->server_fd = server_fd;
        data->client_fd = client_fd;
        data->fps = 5; // TODO

        (*socket_connect_count)++;
        if (pthread_create(&thread, NULL,
                           (void *(*)(void *))on_connect, data)) {
            die("ptherad_create() failed");
        }

        if (pthread_join(thread, NULL)) {
            die("pthread_join() failed");
        }
        (*socket_connect_count)--;

        close(client_fd);

        log("client closed. port = %d", port);
    }

    close(server_fd);

    return NULL;
}

static void listen_socket(const int port, const OnConnect on_connect, int *socket_connect_count)
{
    pthread_t thread;
    ListenSocketData *thread_data
        = (ListenSocketData *)malloc(sizeof(ListenSocketData));
    thread_data->port = port;
    thread_data->on_connect = on_connect;
    thread_data->socket_connect_count = socket_connect_count;

    if (pthread_create(&thread, NULL, listen_socket_func, thread_data)) {
        die("pthread_create() failed!");
    }

    if (pthread_detach(thread)) {
        die("pthread_detach() failed");
    }
}

static void broadcast_discovery_packet(const int port,
                                       const int *socket_connect_count)
{
    int sock;
    int broadcast_enable = 1;
    int ret;
    struct sockaddr_in sin;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        die("socket() failed");
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                     &broadcast_enable, sizeof(broadcast_enable));
    if (ret == -1) {
        die("setsockopt() failed");
    }

    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = (in_port_t)htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    while (true) {
        if (*socket_connect_count == 0) {
            char msg[DISCOVERY_PACKET_SIZE] = {0,};

            log("broadcasting discovery packet...");
            strncpy(msg, "NX_REMOTE|1.0.0|NX500|", sizeof(msg)); // HEADER|VERSION|MODEL|

            if (sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&sin,
                       sizeof(struct sockaddr_in)) == -1) {
                print_error("sendto() failed");
                break;
            }
        }
        sleep(1);
    }

    if (close(sock) == -1) {
        print_error("close() failed");
    }
}

int main(int argc, char **argv)
{
    int socket_connect_count = 0;

    signal(SIGPIPE, SIG_IGN);

    listen_socket(PORT_VIDEO, start_video_capture, &socket_connect_count);
    listen_socket(PORT_XWIN, start_xwin_capture, &socket_connect_count);
    listen_socket(PORT_EXECUTOR, start_executor, &socket_connect_count);

    broadcast_discovery_packet(PORT_UDP_BROADCAST, &socket_connect_count);

    return 0;
}
