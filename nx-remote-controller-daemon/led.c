#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#define SYS_LED_GPIO_PATH "/sys/class/leds/card/brightness"

void led_set(bool on)
{
    int fd;
    int ret;

    fd = open(SYS_LED_GPIO_PATH, O_WRONLY);
    if (fd == -1) {
        print_error("open() failed");
        return;
    }


    if (on) {
        ret = write(fd, "255", 3);
    } else {
        ret = write(fd, "0", 1);
    }
    if (ret == -1) {
        print_error("write() failed");
    }

    if (close(fd) == -1) {
        print_error("close() failed");
    }
}
