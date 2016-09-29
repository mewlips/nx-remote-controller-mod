#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#define LCD_CONTROL_SH_COMMAND "lcd_control.sh"
#define NX_INPUT_INJECTOR_COMMAND "nx-input-injector"
#define XEV_NX_COMMAND "xev-nx"
#define XDOTOOL_COMMAND "xdotool"

extern void run_command(char *command_line);
extern const char *get_app_path(void);
extern const char *get_chroot_command(void);
extern int systemf(const char *fmt, ...);

#endif
