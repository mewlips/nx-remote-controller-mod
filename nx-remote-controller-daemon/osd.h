#ifndef OSD_H_INCLUDED
#define OSD_H_INCLUDED

#include <stdbool.h>

#include "mongoose.h"

extern void osd_http_send(struct mg_connection *nc, struct http_message *hm);
extern void osd_init(void);
extern void osd_destroy(void);
extern bool osd_is_evf(void);

#endif
