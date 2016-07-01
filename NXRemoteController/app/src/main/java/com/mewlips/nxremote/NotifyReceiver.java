package com.mewlips.nxremote;

import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Timer;
import java.util.TimerTask;

import static com.mewlips.nxremote.Configurations.*;

/**
 * Created by mewlips on 16. 6. 29.
 */
public class NotifyReceiver extends Thread {
    private static final String TAG = "NotifyReceiver";

    private NXCameraInfo mCameraInfo;
    private MainActivity mActivity;
    private Socket mSocket;
    private BufferedReader mReader;

    public NotifyReceiver(NXCameraInfo cameraInfo, MainActivity activity) {
        mCameraInfo = cameraInfo;
        mActivity = activity;
    }
    @Override
    public void run() {
        if (mCameraInfo == null || mCameraInfo.getIpAddress() == null) {
            return;
        }
        try {
            mSocket = new Socket(mCameraInfo.getIpAddress(), Configurations.NOTIFY_PORT);
            mReader = new BufferedReader(new InputStreamReader(mSocket.getInputStream()));
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        ConnectionChecker connectionChecker = new ConnectionChecker();

        while (mSocket != null) {
            try {
                if (mReader == null) {
                    if (mSocket != null) {
                        closeSocket();
                    }
                    break;
                }
                String line = mReader.readLine();
                if (line == null) {
                    Log.d(TAG, "line is null");
                    closeSocket();
                    break;
                } else if (line.startsWith(PING_COMMAND)) {
                    connectionChecker.updatePingTime();
                } else if (line.startsWith("keydown ")) {
                    final String key = line.replace("keydown ", "");
                    mActivity.setButtonBackground(key, true);
                } else if (line.startsWith("keyup ")) {
                    final String key = line.replace("keyup ", "");
                    mActivity.setButtonBackground(key, false);
                } else if (line.startsWith("hevc=on")) {
                    if (!mCameraInfo.isRecording()) {
                        Log.d(TAG, "on record.");
                        mCameraInfo.setRecording(true);
                        mActivity.onRecordStarted();
                    }
                } else if (line.startsWith("hevc=off")) {
                    if (mCameraInfo.isRecording()) {
                        Log.d(TAG, "record stopped.");
                        mCameraInfo.setRecording(false);
                        mActivity.onRecordStopped();
                    }
                } else if (line.startsWith("socket_closed=")) {
                    String type = line.replace("socket_closed=", "");
                    if (type.startsWith("video")) {
                        mActivity.startVideoPlayer();
                    } else if (type.startsWith("xwin")) {
                        mActivity.startXWinViewer();
                    } else if (type.startsWith("executor")) {
                        mActivity.startExecutor();
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
                Log.d(TAG, "notify read failed.");
                if (mSocket != null) {
                    closeSocket();
                }
            }
        }
        connectionChecker.cancel();
    }

    protected void closeSocket() {
        if (mReader != null) {
            try {
                mReader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mReader = null;
        }
        if (mSocket != null) {
            try {
                mSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mSocket = null;
        }
    }

    private class ConnectionChecker {
        private Timer mTimer;
        private long mLastPingTime;

        public ConnectionChecker() {
            updatePingTime();

            mTimer = new Timer();
            mTimer.scheduleAtFixedRate(new TimerTask() {
                @Override
                public void run() {
                    long currentTime = System.currentTimeMillis();
                    if (mLastPingTime < currentTime - 2000) {
                        Log.i(TAG, "nx-remote-controller-dameon is not responding.");
                        mActivity.disconnectFromCameraDaemon();
                        mActivity.startDiscovery();
                        mTimer.cancel();
                        mTimer.purge();
                    }
                }
            }, 2000, 2000);
        }

        public void updatePingTime() {
            mLastPingTime = System.currentTimeMillis();
        }

        public void cancel() {
            mTimer.cancel();
            mTimer.purge();
        }
    }

}
