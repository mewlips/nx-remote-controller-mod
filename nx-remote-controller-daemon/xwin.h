#ifndef XWIN_H_INCLUDED
#define XWIN_H_INCLUDED

#include "mongoose.h"
#include "network.h"

extern void xwin_init(void);
extern void xwin_set_fps(int fps);
extern void *xwin_start_capture(Sockets *sockets);
extern void xwin_stop_capture(void);

#endif
