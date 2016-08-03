#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include <stdbool.h>

#include "mongoose.h"
#include "network.h"

extern void video_init(void);
extern void video_set_fps(int fps);
extern void video_set_evf(bool on);
extern void *video_start_capture(Sockets *sockets);
extern void video_stop_capture(void);

#endif
