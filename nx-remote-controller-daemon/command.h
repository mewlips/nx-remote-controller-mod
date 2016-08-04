#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

extern void run_command(char *command_line);
extern const char *get_app_path(void);
extern const char *get_chroot_command(void);
extern int systemf(const char *fmt, ...);

#endif
