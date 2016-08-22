#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

extern void input_init(void);
extern void input_destroy(void);
extern void input_inject(const char *command);
extern void input_inject_keep_alive(void);

#endif
