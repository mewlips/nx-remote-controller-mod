#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdio.h>

#ifdef DEBUG
#define print_log(fmt, ...) \
    do { \
        fprintf(stderr, "[%s():%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while(0)
#else
#define print_log(fmt, ...)
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define PING_TIMEOUT_MS 5000

extern void die(const char *msg);
extern void print_error(const char *msg);
extern long long get_current_time(void);

#endif
