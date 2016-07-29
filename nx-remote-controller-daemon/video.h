#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "mongoose.h"
#include "network.h"

extern void *mmap_lcd(const int fd, const off_t offset);
extern void munmap_lcd(void *addr, const off_t offset);
extern void init_video(void);
extern void destroy_video(void);
extern void set_video_fps(int fps);
extern void set_video_evf(bool on);
extern void *start_video_capture(Sockets *sockets);
extern void stop_video_capture(void);
extern void send_video(struct mg_connection *nc, struct http_message *hm);

#endif
