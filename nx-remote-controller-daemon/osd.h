#ifndef OSD_H_INCLUDED
#define OSD_H_INCLUDED

extern void osd_send(struct mg_connection *nc, struct http_message *hm);
extern void osd_init(void);
extern void osd_destroy(void);

#endif
