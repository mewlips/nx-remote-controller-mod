package com.mewlips.nxremote;

import android.os.Build;
import android.util.Log;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import static com.mewlips.nxremote.Configurations.*;

/**
 * Created by mewlips on 16. 6. 29.
 */
public class CommandExecutor extends Thread {
    private static final String TAG = "CommandExecutor";

    private NXCameraInfo mCameraInfo;
    private MainActivity mActivity;
    private Socket mSocket;
    private OutputStreamWriter mWriter;
    private DataInputStream mInputStream;

    private BlockingQueue<String> mBlockingQueue = new ArrayBlockingQueue<>(1000);

    public CommandExecutor(NXCameraInfo cameraInfo, MainActivity activity) {
        mCameraInfo = cameraInfo;
        mActivity = activity;
    }

    @Override
    public void run() {
        if (mCameraInfo == null || mCameraInfo.getIpAddress() == null) {
            return;
        }
        try {
            mSocket = new Socket(mCameraInfo.getIpAddress(), EXECUTOR_PORT);
            mWriter = new OutputStreamWriter(mSocket.getOutputStream());
            mInputStream = new DataInputStream(mSocket.getInputStream());
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        execute(POPUP_TIMEOUT_SH_COMMAND + " 3 Connected to " + Build.MODEL +
                " (" + mActivity.getWifiIpAddress() + ")");

        Timer timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                execute("ping");
            }
        }, 1000, 1000);

        while (mSocket != null) {
            try {
                String command = mBlockingQueue.take();
                byte[] readBuf = new byte[1024];

                if (!command.startsWith(INJECT_INPUT_COMMAND) &&
                        !command.startsWith(PING_COMMAND)) {
                    Log.d(TAG, "command = " + command);
                }
                try {
                    if (mWriter == null || mInputStream == null) {
                        if (mSocket != null) {
                            closeSocket();
                        }
                        break;
                    }
                    mWriter.write(command + "\n");
                    mWriter.flush();

                    String commandOutput = "";
                    while (true) {
                        int size = mInputStream.readInt();
                        if (size > readBuf.length) {
                            Log.e(TAG, "size is too big. size = " + size);
                            break;
                        }
                        if (size > 0) {
                            while (size > 0) {
                                int readSize = mInputStream.read(readBuf, 0, size);
                                size -= readSize;
                                commandOutput += new String(readBuf, 0, readSize);
                            }
                        } else {
                            if (command.equals(GET_MOV_SIZE_COMMAND_NX1) ||
                                    command.equals(GET_MOV_SIZE_COMMAND_NX500)) {
                                Log.d(TAG, "commandOutput = " + commandOutput);
                                if (commandOutput.startsWith("[app] in memory:") &&
                                        commandOutput.length() > 30) {
                                    String output = commandOutput.substring(29);
                                    if (output.startsWith("0")) {
                                        Log.d(TAG, "4096x2160");
                                        mActivity.setVideoSize(VIDEO_SIZE_UHD);
                                    } else if (output.startsWith("9") || output.startsWith("10") || output.startsWith("11")) {
                                        Log.d(TAG, "640x480");
                                        mActivity.setVideoSize(VIDEO_SIZE_VGA);
                                    } else {
                                        Log.d(TAG, "16/9");
                                        mActivity.setVideoSize(VIDEO_SIZE_FHD_HD);
                                    }
                                    mActivity.setVideoMargin();
                                }
                            } else {
                                if (commandOutput.length() > 0) {
                                    Log.d(TAG, "command output (" + commandOutput.length() + ") = " + commandOutput);
                                }
                            }
                            break;
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    Log.d(TAG, "exectuor read or write failed.");
                    if (mSocket != null) {
                        closeSocket();
                    }
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        timer.cancel();
        timer.purge();
    }

    protected void execute(String command) {
        if (mSocket != null) {
            mBlockingQueue.add(command);
        }
    }

    protected void closeSocket() {
        if (mWriter != null) {
            try {
                mWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mWriter = null;
        }
        if (mInputStream != null) {
            try {
                mInputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mInputStream = null;
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
}