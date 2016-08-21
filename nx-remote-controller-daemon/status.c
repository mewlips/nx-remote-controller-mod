#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "nx_model.h"
#include "status.h"
#include "util.h"

static DialMode s_mode = MODE_UNKNOWN;

static int read_int_from_file(const char *path)
{
    int fd;
    char buf[16];
    ssize_t read_size;
    int value = -1;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        print_error("open() failed!");
        goto error;
    }

    memset(buf, 0, sizeof(buf));
    read_size = read(fd, buf, sizeof(buf));
    if (read_size == -1) {
        print_error("read() failed!");
        goto error;
    }

    value = atoi(buf);

error:
    if (fd != -1) {
        close(fd);
    }

    return value;
}

HevcState get_hevc_state(void)
{
    HevcState hevc_state = HEVC_STATE_UNKNOWN;
    int fd;
    char buf[16];
    ssize_t read_size;

    if (is_old_nx_model()) {
        return HEVC_STATE_UNKNOWN;
    }

    fd = open("/sys/kernel/debug/pmu/hevc/state", O_RDONLY);
    if (fd == -1) {
        print_error("open() failed!");
        goto error;
    }

    memset(buf, 0, sizeof(buf));
    read_size = read(fd, buf, sizeof(buf));
    if (read_size == -1) {
        print_error("read() failed!");
        goto error;
    }
    if (strncmp(buf, "on", 2) == 0) {
        hevc_state = HEVC_STATE_ON;
    } else if (strncmp(buf, "off", 3) == 0) {
        hevc_state = HEVC_STATE_OFF;
    }

error:
    if (fd != -1) {
        close(fd);
    }

    return hevc_state;
}

const char *get_hevc_state_string(void)
{
    HevcState state = get_hevc_state();
    switch (state) {
        case HEVC_STATE_UNKNOWN:
            return "unknown";
        case HEVC_STATE_ON:
            return "on";
        case HEVC_STATE_OFF:
            return "off";
    }
    return "unknown";
}

MovieSize get_movie_size(void)
{
    const char *command;
    FILE *command_pipe;
    char buf[256], *p = NULL;

    if (is_nx1()) {
        command = "prefman get 0 0x00000330 l";
    } else if (is_nx500()) {
        command = "prefman get 0 0x0000a360 l";
    } else {
        print_log("only NX1 or NX500");
        return MOVIE_SIZE_UNKNOWN;
    }

    command_pipe = popen(command, "r");
    while (fgets(buf, sizeof(buf), command_pipe) != NULL) {
        if ((p = strstr(buf, "value = ")) != NULL) {
            p += strlen("value = ");
            break;
        }
    }
    pclose(command_pipe);

    if (p != NULL) {
        int value = atoi(p);
        if (is_nx1()) {
            switch (value) {
                case 0:
                    return MOVIE_SIZE_4K;
                case 12:
                case 13:
                case 14:
                    return MOVIE_SIZE_VGA;
                default:
                    return MOVIE_SIZE_UHD_FHD_HD;
            }
        } else if (is_nx500()) {
            switch (value) {
                case 0:
                    return MOVIE_SIZE_4K;
                case 9:
                case 10:
                case 11:
                    return MOVIE_SIZE_VGA;
                default:
                    return MOVIE_SIZE_UHD_FHD_HD;
            }
        }
    } else {
        print_log("prefman failed");
    }

    return MOVIE_SIZE_UNKNOWN;
}

bool is_battery_charging(void)
{
    if (is_nx1()) {
        return read_int_from_file(
                "/sys/devices/platform/micom-ctrl/bat_charging") == 1;
    } else if (is_nx500()) {
        return read_int_from_file("/sys/devices/platform/d5-adc-battery"
                                  "/power_supply/battery/charge_now") == 1;
    } else if (is_old_nx_model()) {
        return read_int_from_file(
                "/sys/devices/platform/jack/charger1_online") == 1;
    }

    return false;
}

int get_battery_percent(void)
{
    if (is_nx1()) {
        return read_int_from_file(
                "/sys/devices/platform/micom-ctrl/body_bat_soc");
    }
    return -1;
}

int get_battery_level(void)
{
    int fd;
    unsigned char buf[32];
    int battery_level = -1;

    if (is_new_nx_model()) {
        fd = open("/var/run/memory/sysman/battery_status_low", O_RDONLY);
        if (fd == -1) {
            print_error("open() failed");
            return -1;
        }

        if (8 == read(fd, buf, sizeof(buf))) {
            battery_level = buf[4];
        } else {
            print_error("read() failed");
        }
    } else {
        fd = open("/var/run/memory/Battery/Capacity", O_RDONLY);
        if (fd == -1) {
            print_error("open() failed");
            return -1;
        }

        if (read(fd, buf, sizeof(buf)) > 16) {
            battery_level = buf[16] - '0';
        } else {
            print_error("read() failed");
        }
    }

    if (close(fd) == -1) {
        print_error("close() failed");
    }

    return battery_level;
}

DialDrive get_dial_drive(void)
{
    if (is_nx1()) {
        return read_int_from_file(
                "/sys/devices/platform/d5keys-polled/dial_drive");
    }
    return DRIVE_UNKNOWN;
}

const char *get_dial_drive_string(void)
{
    DialDrive drive = get_dial_drive();
    switch (drive) {
        case DRIVE_SINGLE:
            return "SINGLE";
        case DRIVE_CONTI_N:
            return "CONTI_N";
        case DRIVE_CONTI_H:
            return "CONTI_H";
        case DRIVE_TIMER:
            return "TIMER";
        case DRIVE_BRACKET:
            return "BRACET";
        default:
            return "UNKNOWN";
    }
}

void set_dial_mode(DialMode mode)
{
    s_mode = mode;
}

DialMode get_dial_mode(void)
{
    if (s_mode == MODE_UNKNOWN) {
        if (is_nx1()) {
            s_mode = read_int_from_file(
                        "/sys/devices/platform/d5keys-polled/dial_mode");

        } else if (is_nx500()) {
            s_mode = read_int_from_file(
                        "/sys/devices/platform/d5-keys/dial_mode");
        } else if (is_old_nx_model()) {
            int mode = read_int_from_file(
                        "/sys/devices/platform/d4keys-polled/mode");
            switch (mode) {
                case 0:
                    s_mode = MODE_NX300_LENS_PRIORITY;
                    break;
                case 3:
                    s_mode = MODE_SCENE;
                    break;
                case 2:
                    s_mode = MODE_NX300_WIFI;
                    break;
                case 5:
                    s_mode = MODE_SMART;
                    break;
                case 6:
                    s_mode = MODE_P;
                    break;
                case 7:
                    s_mode = MODE_A;
                    break;
                case 8:
                    s_mode = MODE_S;
                    break;
                case 9:
                    s_mode = MODE_M;
                    break;
                default:
                    return MODE_UNKNOWN;
            }
        }
    }

    return s_mode;
}

const char *get_dial_mode_string(void)
{
    DialMode mode = get_dial_mode();

    switch (mode) {
        case MODE_SCENE:
            if (is_old_nx_model()) {
                return "Smart";
            } else {
                return "Scene";
            }
        case MODE_SMART:
            if (is_old_nx_model()) {
                return "Smart Auto";
            } else {
                return "Auto";
            }
        case MODE_P:
            return "P";
        case MODE_A:
            return "A";
        case MODE_S:
            return "S";
        case MODE_M:
            return "M";
        case MODE_C1:
            return "C1";
        case MODE_C2:
            return "C2";
        case MODE_SAS:
            return "SAS";
        case MODE_NX300_LENS_PRIORITY:
            return "Lens Priority";
        case MODE_NX300_WIFI:
            return "Wi-Fi";
        default:
            return "UNKNOWN";
    }
}
