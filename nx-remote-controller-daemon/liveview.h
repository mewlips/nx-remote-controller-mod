#ifndef LIVEVIEW_H_INCLUDED
#define LIVEVIEW_H_INCLUDED

#include "mongoose.h"

extern void init_liveview(void);
extern void destroy_liveview(void);
extern void send_liveview(struct mg_connection *nc, struct http_message *hm);

#endif
