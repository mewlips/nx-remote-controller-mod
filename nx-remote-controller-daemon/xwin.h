#ifndef XWIN_H_INCLUDED
#define XWIN_H_INCLUDED

#include "network.h"

extern void init_xwin(int di_camera_app_id);
extern void set_xwin_fps(int fps);
extern void *start_xwin_capture(Sockets *sockets);
extern void stop_xwin_capture(void);

#endif
