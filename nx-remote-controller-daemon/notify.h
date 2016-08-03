#ifndef NOTIFY_H_INCLUDED
#define NOTIFY_H_INCLUDED

#include "network.h"

extern void notify_video_socket_closed(void);
extern void notify_xwin_socket_closed(void);
extern void notify_executor_socket_closed(void);
extern void notify_evf_start(void);
extern void notify_evf_end(void);
extern void *notify_start(Sockets *sockets);

#endif
