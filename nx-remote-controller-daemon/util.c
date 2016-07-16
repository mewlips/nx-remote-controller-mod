#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_error(const char *msg)
{
    perror(msg);
}

long long get_current_time(void)
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
