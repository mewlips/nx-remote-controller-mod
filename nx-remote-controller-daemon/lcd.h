#ifndef LCD_H_INCLUDED
#define LCD_H_INCLUDED

typedef enum {
    LCD_ON,
    LCD_OFF,
    LCD_VIDEO
} LcdState;

extern void lcd_set_state(LcdState state);
extern LcdState lcd_get_state(void);

#endif
