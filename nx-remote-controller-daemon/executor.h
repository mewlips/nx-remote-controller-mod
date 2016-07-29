#ifndef EXECUTOR_H_INCLUDED
#define EXECUTOR_H_INCLUDED

#include "network.h"

extern void *start_executor(Sockets *data);
extern void init_executor(void);
extern void destroy_executor(void);
extern void inject_input(const char *command);

#endif
