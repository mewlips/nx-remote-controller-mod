#include <stdio.h>

#include "command.h"
#include "lcd.h"
#include "util.h"

#define LCD_CONTROL_SH_COMMAND "lcd_control.sh"

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
        systemf("%s/%s %s", get_app_path(), LCD_CONTROL_SH_COMMAND, state_str);
    } else {
        print_log("unknown state. %d", state);
    }
}

LcdState lcd_get_state(void)
{
    return s_lcd_state;
}
