#ifndef XWIN_H_INCLUDED
#define XWIN_H_INCLUDED

#include "mongoose.h"
#include "network.h"

extern void init_xwin(void);
extern void set_xwin_fps(int fps);
extern void *start_xwin_capture(Sockets *sockets);
extern void stop_xwin_capture(void);
extern void send_osd(struct mg_connection *nc, struct http_message *hm);

#endif
