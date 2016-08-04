#include <stdbool.h>
#include <stdlib.h>

#include "util.h"

void shutter_set_silent(bool silent)
{
    print_log("silent shutter = %s", silent ? "on" : "off");

    if (silent) {
        system("/usr/bin/st cap capdtm setusr ADJUSTSHUTTERTYPE 0x750001");
    } else {
        system("/usr/bin/st cap capdtm setusr ADJUSTSHUTTERTYPE 0x750000");
    }
}
