package com.mewlips.nxremote;

/**
 * Created by mewlips on 16. 6. 29.
 */
public class Configurations {
    public static final int NOTIFY_PORT = 5677;
    public static final int VIDEO_STREAMER_PORT = 5678;
    public static final int XWIN_STREAMER_PORT = 5679;
    public static final int EXECUTOR_PORT = 5680;
    public static final int DISCOVERY_UDP_PORT = 5681;

    public static final int FRAME_WIDTH = 720;
    public static final int FRAME_HEIGHT = 480;
    public static final int FRAME_VIDEO_SIZE = FRAME_WIDTH * FRAME_HEIGHT * 3 / 2; // NV12

    public static final int OLD_NX_LCD_WIDTH = 480;
    public static final int OLD_NX_LCD_HEIGHT = 800;
    public static final int OLD_NX_FRAME_WIDTH = 800;
    public static final int OLD_NX_FRAME_VIDEO_SIZE = OLD_NX_FRAME_WIDTH * FRAME_HEIGHT * 3 / 2;

    public static final int XWIN_SEGMENT_NUM_PIXELS = 320;
    public static final int XWIN_SEGMENT_SIZE = 2 + (XWIN_SEGMENT_NUM_PIXELS * 4); // 2 bytes (INDEX) + 320 pixels (BGRA)
    public static final int DISCOVERY_PACKET_SIZE = 32;

    public static final int VIDEO_SIZE_NORMAL = -1;
    public static final int VIDEO_SIZE_FHD_HD = 0;
    public static final int VIDEO_SIZE_UHD = 1;
    public static final int VIDEO_SIZE_VGA = 2;

    public static final String APP_PATH = "/opt/usr/apps/nx-remote-controller-mod";
    public static final String INJECT_INPUT_COMMAND = "inject_input=";
    public static final String PING_COMMAND = "ping";
    public static final String MOD_GUI_COMMAND_NX500
            = "@/opt/usr/nx-on-wake/EV_EV.sh";
    public static final String POPUP_TIMEOUT_SH_COMMAND
            = "@" + APP_PATH + "/popup_timeout.sh";
    public static final String GET_MOV_SIZE_COMMAND_NX1
            = "$prefman get 0 0x00000330 l";
    public static final String GET_MOV_SIZE_COMMAND_NX500
            = "$prefman get 0 0x0000a360 l";
    public static final String LCD_ON_COMMAND = "lcd=on";
    public static final String LCD_OFF_COMMAND = "lcd=off";
    public static final String LCD_VIDEO_COMMAND = "lcd=video";
    public static final String LCD_OSD_COMMAND = "lcd=osd";
}
