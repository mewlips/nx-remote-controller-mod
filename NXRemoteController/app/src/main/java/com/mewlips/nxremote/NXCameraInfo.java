package com.mewlips.nxremote;

/**
 * Created by mewlips on 16. 6. 29.
 */
public class NXCameraInfo {
    private String daemonVersion;
    private String model;
    private String ipAddress;
    private boolean onRecording;

    public NXCameraInfo(String daemonVersion, String model, String ipAddress) {
        this.daemonVersion = daemonVersion;
        this.model = model;
        this.ipAddress = ipAddress;
    }

    public String getDaemonVersion() {
        return daemonVersion;
    }

    public String getModel() {
        return model;
    }

    public String getIpAddress() {
        return ipAddress;
    }

    public boolean isRecording() {
        return onRecording;
    }

    public void setRecording(boolean onRecording) {
        this.onRecording = onRecording;
    }

    @Override
    public String toString() {
        return "NXCameraInfo{" +
                "daemonVersion='" + daemonVersion + '\'' +
                ", model='" + model + '\'' +
                ", ipAddress='" + ipAddress + '\'' +
                '}';
    }
}
