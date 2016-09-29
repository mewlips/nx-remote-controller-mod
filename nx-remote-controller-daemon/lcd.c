#include <stdio.h>

#include "command.h"
#include "lcd.h"
#include "nx_model.h"
#include "util.h"

static LcdState s_lcd_state;

void lcd_set_state(LcdState state)
{
    const char *state_str = NULL;

    print_log("state = %d", state);
    switch (state) {
        case LCD_ON:
            state_str = "on";
            s_lcd_state = LCD_ON;
            break;
        case LCD_OFF:
            state_str = "off";
            s_lcd_state = LCD_OFF;
            break;
        case LCD_VIDEO:
            state_str = "video";
            s_lcd_state = LCD_VIDEO;
            break;
    }
    if (state_str != NULL) {
        if (is_old_nx_model()) {
            systemf("%s/%s %s", get_app_path(), LCD_CONTROL_SH_COMMAND, state_str);
        } else {
            systemf("kill $(ps ax | grep 'xev-nx -.. -bi' | grep -v grep | awk '{print $1}')");
            if (state == LCD_OFF || state == LCD_VIDEO) {
                systemf("%s %s %s -bi %lu &",
                        get_chroot_command(), XEV_NX_COMMAND,
                        state == LCD_OFF ? "-rv" : "-tr",
                        get_di_camera_app_window_id());
            }
            systemf("%s %s %s %lu",
                    get_chroot_command(), XDOTOOL_COMMAND,
                    state == LCD_VIDEO ? "windowunmap" : "windowmap",
                    get_di_camera_app_window_id());
        }
    } else {
        print_log("unknown state. %d", state);
    }
}

LcdState lcd_get_state(void)
{
    return s_lcd_state;
}
