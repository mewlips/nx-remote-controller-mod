package com.mewlips.nxremote;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;

import static com.mewlips.nxremote.Configurations.FRAME_HEIGHT;
import static com.mewlips.nxremote.Configurations.FRAME_WIDTH;
import static com.mewlips.nxremote.Configurations.OLD_NX_LCD_HEIGHT;
import static com.mewlips.nxremote.Configurations.OLD_NX_LCD_WIDTH;
import static com.mewlips.nxremote.Configurations.XWIN_SEGMENT_NUM_PIXELS;
import static com.mewlips.nxremote.Configurations.XWIN_SEGMENT_SIZE;
import static com.mewlips.nxremote.Configurations.XWIN_STREAMER_PORT;

/**
 * Created by mewlips on 16. 6. 29.
 */

public class XWinViewer extends Thread {
    private static final String TAG = "XWinViewer";

    private NXCameraInfo mCameraInfo;
    private MainActivity mActivity;
    private Socket mSocket;
    private InputStream mReader;

    private byte[] mBuffer = new byte[XWIN_SEGMENT_SIZE];
    private int[] mIntArray;

    public XWinViewer(NXCameraInfo cameraInfo, MainActivity activity) {
        mCameraInfo = cameraInfo;
        mActivity = activity;

        if (mCameraInfo.isOldNxModel()) {
            mIntArray = new int[OLD_NX_LCD_WIDTH * OLD_NX_LCD_HEIGHT];
        } else {
            mIntArray = new int[FRAME_WIDTH * FRAME_HEIGHT];
        }
    }

    @Override
    public void run() {
        if (mCameraInfo == null || mCameraInfo.getIpAddress() == null) {
            return;
        }
        try {
            mSocket = new Socket(mCameraInfo.getIpAddress(), XWIN_STREAMER_PORT);
            mReader = mSocket.getInputStream();
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        int updateCount = 0;
        while (mSocket != null) {
            int readSize = 0;
            try {
                while (readSize != XWIN_SEGMENT_SIZE) {
                    if (mReader == null) {
                        readSize = -1;
                        break;
                    }
                    readSize += mReader.read(mBuffer, readSize, XWIN_SEGMENT_SIZE - readSize);
                    if (readSize == -1) {
                        break;
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            if (readSize == -1) {
                Log.d(TAG, "xwin read failed.");
                if (mSocket != null) {
                    closeSocket();
                }
                break;
            }
            if (readSize == XWIN_SEGMENT_SIZE) {
                int index = (mBuffer[0] & 0xff) << 8 | mBuffer[1] & 0xff;
//                    Log.d(TAG, "index = " + index);
                if (index == 0x0fff) { // end of frame
                    if (updateCount > 0) {
//                            Log.d(TAG, "update xwin");
                        Bitmap bitmap;
                        if (mCameraInfo.isOldNxModel()) {
                            bitmap = Bitmap.createBitmap(mIntArray, OLD_NX_LCD_WIDTH, OLD_NX_LCD_HEIGHT, Bitmap.Config.ARGB_8888);
                            Matrix matrix = new Matrix();
                            matrix.postRotate(270.0f);
                            Bitmap rotatedBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
                            mActivity.setXWinBitmap(rotatedBitmap);

                        } else if (mCameraInfo.isNx1()) {
                            int x = (mBuffer[2] & 0xff) << 24 | (mBuffer[3] & 0xff)  << 16 |
                                    (mBuffer[4] & 0xff) <<  8 | (mBuffer[5] & 0xff);
                            int y = (mBuffer[6] & 0xff) << 24 | (mBuffer[7] & 0xff)  << 16 |
                                    (mBuffer[8] & 0xff) <<  8 | (mBuffer[9] & 0xff);
                            int w = (mBuffer[10] & 0xff) << 24 | (mBuffer[11] & 0xff)  << 16 |
                                    (mBuffer[12] & 0xff) <<  8 | (mBuffer[13] & 0xff);
                            int h = (mBuffer[14] & 0xff) << 24 | (mBuffer[15] & 0xff)  << 16 |
                                    (mBuffer[16] & 0xff) <<  8 | (mBuffer[17] & 0xff);
                            Log.d(TAG, "x = " + x + ", y = " + y + ",w = " + w + ",h = " + h);
                            if (w != FRAME_WIDTH || h != FRAME_HEIGHT) {
                                bitmap = Bitmap.createBitmap(
                                        FRAME_WIDTH, FRAME_HEIGHT, Bitmap.Config.ARGB_8888);
                                Canvas canvas = new Canvas(bitmap);
                                Bitmap overlay = Bitmap.createBitmap(mIntArray, w, h, Bitmap.Config.ARGB_8888);
                                Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
                                canvas.drawBitmap(overlay, x, y, paint);
                            } else {
                                bitmap = Bitmap.createBitmap(mIntArray, FRAME_WIDTH, FRAME_HEIGHT, Bitmap.Config.ARGB_8888);
                            }
                            mActivity.setXWinBitmap(bitmap);
                        } else {
                            bitmap = Bitmap.createBitmap(mIntArray, FRAME_WIDTH, FRAME_HEIGHT, Bitmap.Config.ARGB_8888);
                            mActivity.setXWinBitmap(bitmap);
                        }
                    }
                    updateCount = 0;
                } else {
                    int offset = index * XWIN_SEGMENT_NUM_PIXELS;
                    for (int i = 0; i < XWIN_SEGMENT_NUM_PIXELS; i++) {
                        int j = 2 + i * 4;
                        int b = mBuffer[j] & 0xff;
                        int g = mBuffer[j+1] & 0xff;
                        int r = mBuffer[j+2] & 0xff;
                        int a = mBuffer[j+3] & 0xff;
                        mIntArray[offset + i] = (a << 24) | (r << 16) | (g << 8) | b;
                    }
                    updateCount++;
                }
            } else {
                Log.d(TAG, "readSize = " + readSize);
            }
        }
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
}