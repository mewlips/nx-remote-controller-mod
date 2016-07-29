#ifndef OSD_H_INCLUDED
#define OSD_H_INCLUDED

extern void send_osd(struct mg_connection *nc, struct http_message *hm);
extern void init_osd(void);
extern void destroy_osd(void);

#endif
