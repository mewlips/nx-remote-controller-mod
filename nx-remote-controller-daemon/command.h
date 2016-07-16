#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

//#define APP_PATH "/opt/usr/apps/nx-remote-controller-mod"
#define APP_PATH "/mnt/mmc/remote"
#define POPUP_TIMEOUT_SH_COMMAND APP_PATH "/popup_timeout.sh"
#define LCD_CONTROL_SH_COMMAND APP_PATH "/lcd_control.sh"
#define NX_INPUT_INJECTOR_COMMAND "chroot " APP_PATH "/tools nx-input-injector"

#define CHROOT_COMMAND \
        "chroot " APP_PATH "/tools "
#define GET_DI_CAMERA_APP_WINDOW_ID_COMMAND \
        "\"$(" CHROOT_COMMAND "xdotool search --class di-camera-app)\""
#define XEV_NX_COMMAND \
        CHROOT_COMMAND "xev-nx -p -id " \
        GET_DI_CAMERA_APP_WINDOW_ID_COMMAND

extern void run_command(char *command_line);

#endif
