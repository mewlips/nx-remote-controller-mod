#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"
#include "util.h"

void gallery_init(void)
{
    char path[256];
    struct stat buf;

    snprintf(path, sizeof(path), "%s/web_root/DCIM", get_app_path());
    if (lstat(path, &buf) != 0) { // link not exist
        symlink("/sdcard/DCIM", path);
    }
}
