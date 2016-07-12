package com.mewlips.nxremote;

import android.graphics.Bitmap;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.util.Arrays;

import static com.mewlips.nxremote.Configurations.*;

/**
 * Created by mewlips on 16. 6. 29.
 */
public class VideoPlayer extends Thread {
    private static final String TAG = "VideoPlayer";

    private NXCameraInfo mCameraInfo;
    private MainActivity mActivity;
    private Socket mSocket;
    private InputStream mReader;

    private byte[] mBuffer;
    private int mVideoSize = 0;
    private final int mFrameVideoSize;

    public VideoPlayer(NXCameraInfo cameraInfo, MainActivity activity) {
        mCameraInfo = cameraInfo;
        mActivity = activity;
        if (cameraInfo.isNewNxModel()) {
            mBuffer = new byte[FRAME_VIDEO_SIZE];
            mFrameVideoSize = FRAME_VIDEO_SIZE;
        } else {
            mBuffer = new byte[OLD_NX_FRAME_VIDEO_SIZE];
            mFrameVideoSize = OLD_NX_FRAME_VIDEO_SIZE;
        }
    }

    @Override
    public void run() {
        if (mCameraInfo == null || mCameraInfo.getIpAddress() == null) {
            return;
        }
        try {
            mSocket = new Socket(mCameraInfo.getIpAddress(), VIDEO_STREAMER_PORT);
            mReader = mSocket.getInputStream();
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        while (mSocket != null) {
            int readSize = 0;
            try {
                while (readSize != mFrameVideoSize) {
                    if (mReader == null) {
                        readSize = -1;
                        break;
                    }
                    readSize += mReader.read(mBuffer, readSize, mFrameVideoSize - readSize);
                    if (readSize == -1) {
                        break;
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            if (readSize == -1) {
                Log.d(TAG, "video read failed.");
                if (mSocket != null) {
                    closeSocket();
                }

                break;
            }
            if (readSize == mFrameVideoSize) {
                int width = mCameraInfo.isNewNxModel() ? FRAME_WIDTH : OLD_NX_FRAME_WIDTH;
                int height = FRAME_HEIGHT;
                if (mVideoSize == VIDEO_SIZE_VGA) {
                    width = 640;
                } else if (mCameraInfo.isRecording() && mVideoSize == VIDEO_SIZE_FHD_HD) {
                    height = 404;
                } else if (mCameraInfo.isRecording() && mVideoSize == VIDEO_SIZE_UHD) {
                    height = 380;
                }
                int[] intArray;
                if (width == 640) {
                    intArray = convertYUV420_NV12toRGB8888(Arrays.copyOfRange(mBuffer, 0, width * height * 3 / 2), width, height);
                } else {
                    intArray = convertYUV420_NV12toRGB8888(mBuffer, width, height);
                }
                Bitmap bitmap = Bitmap.createBitmap(intArray, width, height, Bitmap.Config.ARGB_8888);
                mActivity.setVideoBitmap(bitmap);

            } else {
                Log.d(TAG, "readSize = " + readSize);
            }
        }
    }

    /**
     * Converts YUV420 NV12 to RGB8888
     *
     * @param data byte array on YUV420 NV12 format.
     * @param width pixels width
     * @param height pixels height
     * @return a RGB8888 pixels int array. Where each int is a pixels ARGB.
     */
    public static int[] convertYUV420_NV12toRGB8888(byte [] data, int width, int height) {
        int size = width*height;
        int offset = size;
        int[] pixels = new int[size];
        int u, v, y1, y2, y3, y4;

        // i percorre os Y and the final pixels
        // k percorre os pixles U e V
        for(int i=0, k=0; i < size; i+=2, k+=2) {
            y1 = data[i  ]&0xff;
            y2 = data[i+1]&0xff;
            y3 = data[width+i  ]&0xff;
            y4 = data[width+i+1]&0xff;

            v = data[offset+k  ]&0xff;
            u = data[offset+k+1]&0xff;
            u = u-128;
            v = v-128;

            pixels[i  ] = convertYUVtoRGB(y1, u, v);
            pixels[i+1] = convertYUVtoRGB(y2, u, v);
            pixels[width+i  ] = convertYUVtoRGB(y3, u, v);
            pixels[width+i+1] = convertYUVtoRGB(y4, u, v);

            if (i!=0 && (i+2)%width==0)
                i+=width;
        }

        return pixels;
    }

    private static int convertYUVtoRGB(int y, int u, int v) {
        int r,g,b;

        r = y + (int)1.402f*v;
        g = y - (int)(0.344f*u +0.714f*v);
        b = y + (int)1.772f*u;
        r = r>255? 255 : r<0 ? 0 : r;
        g = g>255? 255 : g<0 ? 0 : g;
        b = b>255? 255 : b<0 ? 0 : b;
        return 0xff000000 | (b<<16) | (g<<8) | r;
    }

    protected void setVideoSize(int videoSize) {
        mVideoSize = videoSize;
    }

    protected int getVideoSize() {
        return mVideoSize;
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