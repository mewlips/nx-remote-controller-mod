#ifndef LIVEVIEW_H_INCLUDED
#define LIVEVIEW_H_INCLUDED

#include "mongoose.h"

extern void *mmap_lcd(const int fd, const off_t offset);
extern void munmap_lcd(void *addr, const off_t offset);
extern void liveview_init(void);
extern void liveview_destroy(void);
extern void liveview_send(struct mg_connection *nc, struct http_message *hm);

#endif
