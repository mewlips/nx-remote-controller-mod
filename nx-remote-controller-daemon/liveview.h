#ifndef LIVEVIEW_H_INCLUDED
#define LIVEVIEW_H_INCLUDED

#include <stdbool.h>

#include "mongoose.h"

extern void liveview_init(void);
extern void liveview_destroy(void);
extern void liveview_http_send(struct mg_connection *nc,
                               struct http_message *hm,
                               bool reduce_size);

#endif
