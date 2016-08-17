#ifndef API_SERVER_H_INCLUDED
#define API_SERVER_H_INCLUDED

#include "mongoose.h"

extern void send_200(struct mg_connection *nc);
extern void send_404(struct mg_connection *nc);
extern void send_500(struct mg_connection *nc);
extern void api_server_run(void);

#endif
