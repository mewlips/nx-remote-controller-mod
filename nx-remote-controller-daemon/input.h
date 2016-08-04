#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

extern void input_init(void);
extern void input_destroy(void);
extern void input_inject(const char *command);
extern void input_set_notify_socket(int client_socket);
extern void input_remove_notify_socket(void);

#endif
