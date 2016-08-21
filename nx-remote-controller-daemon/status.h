#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

typedef enum {
    HEVC_STATE_UNKNOWN = -1,
    HEVC_STATE_OFF = 0,
    HEVC_STATE_ON = 1
} HevcState;

typedef enum {
    MOVIE_SIZE_UNKNOWN = -1,
    MOVIE_SIZE_4K = 0,
    MOVIE_SIZE_UHD_FHD_HD = 1,
    MOVIE_SIZE_VGA = 2
} MovieSize;

typedef enum {
    DRIVE_UNKNOWN = -1,
    DRIVE_SINGLE = 0,
    DRIVE_CONTI_N = 1,
    DRIVE_CONTI_H = 2,
    DRIVE_TIMER = 3,
    DRIVE_BRACKET = 4
} DialDrive;

typedef enum {
    MODE_UNKNOWN = -1,
    MODE_SCENE = 0,
    MODE_SMART = 1,
    MODE_P = 2,
    MODE_A = 3,
    MODE_S = 4,
    MODE_M = 5,
    MODE_C1 = 6,
    MODE_C2 = 7,
    MODE_SAS = 8,
    MODE_NX300_LENS_PRIORITY = 100,
    MODE_NX300_WIFI = 101 
} DialMode;

extern HevcState get_hevc_state(void);
extern const char *get_hevc_state_string(void);
extern MovieSize get_movie_size(void);
extern bool is_battery_charging(void);
extern int get_battery_percent(void);
extern int get_battery_level(void);
extern void set_dial_mode(DialMode mode);
extern DialDrive get_dial_drive(void);
extern const char *get_dial_drive_string(void);
extern DialMode get_dial_mode(void);
extern const char *get_dial_mode_string(void);

#endif
