#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "network.h"

extern void init_video(void);
extern void set_video_fps(int fps);
extern void *start_video_capture(Sockets *sockets);
extern void stop_video_capture(void);

#endif
