#ifndef NX_MODEL_H_INCLUDED
#define NX_MODEL_H_INCLUDED

#include <stdbool.h>
#include <sys/types.h>

typedef enum {
    NX_MODEL_UNKNOWN = 0,

    // new NX models
    NX_MODEL_NX1     = 1,
    NX_MODEL_NX500   = 500,

    // old NX models
    NX_MODEL_NX300   = 300,
    NX_MODEL_NX300M  = 301,
    NX_MODEL_NX2000  = 2000,
    NX_MODEL_NX3000  = 3000,
} NxModel;

extern void nx_model_init(void);
extern NxModel get_nx_model(void);
extern const char *get_nx_model_name(void);
extern const char *get_nx_model_version(void);
extern bool is_new_nx_model(void);
extern bool is_old_nx_model(void);
extern bool is_nx1(void);
extern bool is_nx500(void);
extern int get_frame_width(void);
extern int get_frame_height(void);
extern int get_frame_size(void);
extern int get_default_video_fps(void);
extern int get_default_xwin_fps(void);
extern off_t get_video_addr(int index);
extern int get_num_video_addrs(void);
extern size_t get_mmap_video_size(void);
extern const char *get_mac_address(void);

#endif
