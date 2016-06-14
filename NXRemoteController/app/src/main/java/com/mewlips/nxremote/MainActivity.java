package com.mewlips.nxremote;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.lukedeighton.wheelview.WheelView;
import com.lukedeighton.wheelview.adapter.WheelAdapter;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class MainActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {

    private static final String TAG = "MainActivity";
    private static final boolean DEBUG = true;

    private static final String STREAMER_IP = "192.168.0.252"; // TODO:
    //private static final String STREAMER_IP = "192.168.43.150"; // TODO:
    private static final int VIDEO_STREAMER_PORT = 5678;
    private static final int XWIN_STREAMER_PORT = 5679;
    private static final int EXECUTOR_PORT = 5680;
    private static final int FRAME_WIDTH = 720;
    private static final int FRAME_HEIGHT = 480;
    private static final int FRAME_VIDEO_SIZE = FRAME_WIDTH * FRAME_HEIGHT * 3 / 2; // NV12
    private static final int XWIN_SEGMENT_NUM_PIXELS = 320;
    private static final int XWIN_SEGMENT_SIZE = 2 + (XWIN_SEGMENT_NUM_PIXELS * 4); // 2 bytes (INDEX) + 320 pixels (BGRA)

    private ImageView mImageViewVideo;
    private ImageView mImageViewXWin;

    private ModeWheelAdapter mModeWheelAdapter;
    private WheelView mWheelViewMode;

    private JogWheelAdapter mJogWheelAdapterJog1;
    private WheelView mWheelViewJog1;

    private JogWheelAdapter mJogWheelAdapterJog2;
    private WheelView mWheelViewJog2;

    private Socket mVideoSocket;
    private InputStream mVideoReader;
    
    private Socket mXWinSocket;
    private InputStream mXWinReader;

    private Socket mExecutorSocket;
    private OutputStreamWriter mExecutorWriter;
    private CommandExecutor mCommandExecutor;

    private class VideoPlayer implements Runnable {
        private byte[] mBuffer = new byte[FRAME_VIDEO_SIZE];
        @Override
        public void run() {
            try {
                mVideoSocket = new Socket(STREAMER_IP, VIDEO_STREAMER_PORT);
                mVideoReader = mVideoSocket.getInputStream();
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }

            while (true) {
                int readSize = 0;
                try {
                    while (readSize != FRAME_VIDEO_SIZE) {
                        if (mVideoReader == null) {
                            readSize = -1;
                            break;
                        }
                        readSize += mVideoReader.read(mBuffer, readSize, FRAME_VIDEO_SIZE - readSize);
                        if (readSize == -1) {
                            break;
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                if (readSize == -1) {
                    Log.d(TAG, "socket closed.");
                    try {
                        mVideoSocket.close();
                        mVideoSocket = null;
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    break;
                }
                if (readSize == FRAME_VIDEO_SIZE) {
                    int[] mIntArray = convertYUV420_NV12toRGB8888(mBuffer, FRAME_WIDTH, FRAME_HEIGHT);
                    final Bitmap bmp = Bitmap.createBitmap(mIntArray, FRAME_WIDTH, FRAME_HEIGHT, Bitmap.Config.ARGB_8888);

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mImageViewVideo.setImageBitmap(bmp);
                        }
                    });
                } else {
                    Log.d(TAG, "readSize = " + readSize);
                }
            }
        }
    };

    private class XWinViewer implements Runnable {
        private byte[] mBuffer = new byte[XWIN_SEGMENT_SIZE];
        final int[] mIntArray = new int[FRAME_WIDTH * FRAME_HEIGHT];

        @Override
        public void run() {
            try {
                mXWinSocket = new Socket(STREAMER_IP, XWIN_STREAMER_PORT);
                mXWinReader = mXWinSocket.getInputStream();
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }

            int updateCount = 0;
            while (true) {
                int readSize = 0;
                try {
                    while (readSize != XWIN_SEGMENT_SIZE) {
                        if (mXWinReader == null) {
                            readSize = -1;
                            break;
                        }
                        readSize += mXWinReader.read(mBuffer, readSize, XWIN_SEGMENT_SIZE - readSize);
                        if (readSize == -1) {
                            break;
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                if (readSize == -1) {
                    Log.d(TAG, "socket closed.");
                    try {
                        mXWinSocket.close();
                        mXWinSocket = null;
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    break;
                }
                if (readSize == XWIN_SEGMENT_SIZE) {
                    int index = (mBuffer[0] & 0xff) << 8 | mBuffer[1] & 0xff;
//                    Log.d(TAG, "index = " + index);
                    if (index == 0x0fff) { // end of frame
                        if (updateCount > 0) {
//                            Log.d(TAG, "update xwin");
                            final Bitmap bmp = Bitmap.createBitmap(mIntArray, FRAME_WIDTH, FRAME_HEIGHT, Bitmap.Config.ARGB_8888);
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    mImageViewXWin.setImageBitmap(bmp);
                                }
                            });
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
    };

    private class CommandExecutor implements Runnable {
        private BlockingQueue<String> mBlockingQueue = new ArrayBlockingQueue<>(50);

        @Override
        public void run() {
            try {
                mExecutorSocket = new Socket(STREAMER_IP, EXECUTOR_PORT);
                mExecutorWriter = new OutputStreamWriter(mExecutorSocket.getOutputStream());
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }

            while (true) {
                try {
                    String command = mBlockingQueue.take();
                    Log.d(TAG, "command = " + command);
                    try {
                        mExecutorWriter.write(command + "\n");
                        mExecutorWriter.flush();
                    } catch (IOException e) {
                        e.printStackTrace();
                        try {
                            mExecutorSocket.close();
                        } catch (IOException e1) {
                            e1.printStackTrace();
                        }
                        mExecutorSocket = null;
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        public void execute(String command) {
            if (mExecutorSocket != null) {
                mBlockingQueue.add(command);
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
//        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
//        setSupportActionBar(toolbar);

//        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
//        fab.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
//                        .setAction("Action", null).show();
//            }
//        });

//        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
//        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
//                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
//        drawer.setDrawerListener(toggle);
//        toggle.syncState();

        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);

        View.OnTouchListener onTouchListener = new View.OnTouchListener() {
            private static final int SKIP_MOUSE_MOVE_COUNT = 10;
            private int mSkipCount;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction() & MotionEvent.ACTION_MASK;
                int x = (int) (event.getX() * (float)FRAME_WIDTH / (float)v.getWidth());
                int y = (int) (event.getY() * (float)FRAME_HEIGHT / (float)v.getHeight());
                String command = "/opt/usr/scripts/chroot.sh xdotool ";

                Log.d(TAG, "action = " + action + ", x = " + x + ", y = " + y);

                switch (action) {
                    case MotionEvent.ACTION_DOWN:
                        mSkipCount = SKIP_MOUSE_MOVE_COUNT;
                        runCommand(command + "mousemove " + x + " " + y + " mousedown 1");
                        return true;
                    case MotionEvent.ACTION_MOVE:
                        mSkipCount--;
                        if (mSkipCount == 0) {
                            runCommand(command + "mousemove " + x + " " + y);
                            mSkipCount = SKIP_MOUSE_MOVE_COUNT;
                        }
                        return true;
                    case MotionEvent.ACTION_UP:
                        runCommand(command + "mousemove " + x + " " + y + " mouseup 1");
                        break;
                }
                return false;
            }
        };

        int[] intArray = new int[FRAME_WIDTH * FRAME_HEIGHT];
        Bitmap bmp = Bitmap.createBitmap(intArray, FRAME_WIDTH, FRAME_HEIGHT, Bitmap.Config.ARGB_8888);

        mImageViewVideo = (ImageView) findViewById(R.id.imageViewVideo);
        mImageViewVideo.setOnTouchListener(onTouchListener);
        mImageViewVideo.setImageBitmap(bmp);

        mImageViewXWin = (ImageView) findViewById(R.id.imageViewXWin);
        mImageViewXWin.setOnTouchListener(onTouchListener);
        mImageViewXWin.setImageBitmap(bmp);

        mWheelViewMode = (WheelView) findViewById(R.id.wheelViewMode);
        mModeWheelAdapter = new ModeWheelAdapter();
        mWheelViewMode.setAdapter(mModeWheelAdapter);

        mWheelViewMode.setOnWheelItemSelectedListener(new WheelView.OnWheelItemSelectListener() {
            @Override
            public void onWheelItemSelected(WheelView parent,  Drawable itemDrawable, int position) {
                //Log.d(TAG, "angle = " + mWheelViewMode.getAngleForPosition(position));
                mModeWheelAdapter.setSelectedPosition(position);
                Log.d(TAG, "position = " + position + ", " + mModeWheelAdapter.getModeOfSelectedPosition());
            }
        });
        mWheelViewMode.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction() & MotionEvent.ACTION_MASK;
                if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL) {
                    mWheelViewMode.setSelected(mModeWheelAdapter.getSelectedPosition());
                    //mWheelViewMode.setAngle(mWheelViewMode.getAngleForPosition(mModeWheelAdapter.getSelectedPosition()));
                    Log.d(TAG, "onTouch, position = " + mModeWheelAdapter.getSelectedPosition() + ", " + mModeWheelAdapter.getModeOfSelectedPosition());
                    String keyCode = mModeWheelAdapter.getKeyCodeOfSelectedPosition();
                    runCommand("/opt/usr/scripts/chroot.sh xdotool key " + keyCode);
                }
                return false;
            }
        });

        mWheelViewJog1 = (WheelView) findViewById(R.id.wheelViewJog1);
        mJogWheelAdapterJog1 = new JogWheelAdapter();
        mWheelViewJog1.setAdapter(mJogWheelAdapterJog1);
        mWheelViewJog1.setOnWheelItemSelectedListener(new WheelView.OnWheelItemSelectListener() {
            private int mPrevPosition = 0;
            @Override
            public void onWheelItemSelected(WheelView parent,  Drawable itemDrawable, int position) {
                String key;
                if (mPrevPosition == 0 && position == mJogWheelAdapterJog1.getCount() - 1) {
                    key = KEY_JOG1_CW;
                } else if (mPrevPosition == mJogWheelAdapterJog1.getCount() -1 && position == 0) {
                    key = KEY_JOG1_CCW;
                } else if (mPrevPosition > position) {
                    key = KEY_JOG1_CW;
                } else {
                    key = KEY_JOG1_CCW;
                }
                runCommand("/opt/usr/scripts/chroot.sh xdotool key " + key);
                mPrevPosition = position;
            }
        });

        mWheelViewJog2 = (WheelView) findViewById(R.id.wheelViewJog2);
        mJogWheelAdapterJog2 = new JogWheelAdapter();
        mWheelViewJog2.setAdapter(mJogWheelAdapterJog2);
        mWheelViewJog2.setOnWheelItemSelectedListener(new WheelView.OnWheelItemSelectListener() {
            private int mPrevPosition = 0;
            @Override
            public void onWheelItemSelected(WheelView parent,  Drawable itemDrawable, int position) {
                String key;
                if (mPrevPosition == 0 && position == mJogWheelAdapterJog1.getCount() - 1) {
                    key = KEY_JOG_CW;
                } else if (mPrevPosition == mJogWheelAdapterJog1.getCount() - 1 && position == 0) {
                    key = KEY_JOG_CCW;
                } else if (mPrevPosition > position) {
                    key = KEY_JOG_CW;
                } else {
                    key = KEY_JOG_CCW;
                }
                runCommand("/opt/usr/scripts/chroot.sh xdotool key " + key);
                mPrevPosition = position;
            }
        });
    }

    private void startVideoPlayer() {
        if (mVideoSocket == null) {
            new Thread(new VideoPlayer()).start();
        }
        mImageViewVideo.setVisibility(View.VISIBLE);
    }
    private void startXWinViewer() {
        if (mXWinSocket == null) {
            new Thread(new XWinViewer()).start();
        }
        mImageViewXWin.setVisibility(View.VISIBLE);
    }
    @Override
    protected void onResume() {
        super.onResume();

        // TODO: setting
        startVideoPlayer();
        startXWinViewer();
        mCommandExecutor = new CommandExecutor();
        new Thread(mCommandExecutor).start();
    }

    @Override
    protected void onPause() {
        super.onPause();

        closeSockets();
    }

    @Override
    public void onBackPressed() {
        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @SuppressWarnings("StatementWithEmptyBody")
    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        int id = item.getItemId();

        if (id == R.id.nav_full_remote) {
            startVideoPlayer();
            startXWinViewer();
        } else if (id == R.id.nav_without_live_view) {
            closeVideoSocket();
            startXWinViewer();
        } else if (id == R.id.nav_live_view_only) {
            closeXWinSocket();
            startVideoPlayer();
        } else if (id == R.id.nav_buttons_only) {
            closeVideoSocket();
            closeXWinSocket();
            mImageViewVideo.setVisibility(View.GONE);
            mImageViewXWin.setVisibility(View.GONE);
        } else if (id == R.id.nav_lcd_on_off) {
            runCommand("/opt/usr/scripts/lcd_toggle.sh");
        } else if (id == R.id.nav_silent_shutter) {
            runCommand("/opt/usr/scripts/rolling_shutter.sh");
        }

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    private void closeVideoSocket() {
        if (mVideoReader != null) {
            try {
                mVideoReader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mVideoReader = null;
        }
        if (mVideoSocket != null) {
            try {
                mVideoSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        mImageViewVideo.setVisibility(View.INVISIBLE);
    }

    private void closeXWinSocket() {
        if (mXWinReader != null) {
            try {
                mXWinReader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mXWinReader = null;
        }
        if (mXWinSocket != null) {
            try {
                mXWinSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        mImageViewXWin.setVisibility(View.INVISIBLE);
    }

    private void closeSockets() {
        closeVideoSocket();
        closeXWinSocket();

        if (mExecutorWriter != null) {
            try {
                mExecutorWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mExecutorWriter = null;
        }
        if (mExecutorSocket != null) {
            try {
                mExecutorSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        mCommandExecutor = null;
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

    private static final String KEY_UP = "KP_Up";
    private static final String KEY_LEFT = "KP_Left";
    private static final String KEY_RIGHT = "KP_Right";
    private static final String KEY_DOWN = "KP_Down";
    private static final String KEY_DEL = "KP_Delete";
    private static final String KEY_DEPTH = "Henkan_Mode";
    private static final String KEY_METER = "Hiragana_Katakana";
    private static final String KEY_OK = "KP_Enter";
    private static final String KEY_PWON = "XF86AudioRaiseVolume";
    private static final String KEY_PWOFF = "XF86PowerOff";
    private static final String KEY_RESET = "XF86PowerOff";
    private static final String KEY_S1 = "Super_L";
    private static final String KEY_S2 = "Super_R";
    private static final String KEY_MENU = "Menu";
    private static final String KEY_AEL = "XF86Favorites";
    private static final String KEY_REC = "XF86WebCam";
    private static final String KEY_FN = "XF86HomePage";
    private static final String KEY_EV = "XF86Reload";
    private static final String KEY_PB = "XF86Tools";
    private static final String KEY_AF_MODE = "Xf86TaskPane";
    private static final String KEY_WB = "XF86Launch6";
    private static final String KEY_ISO = "XF86Launch7";
    private static final String KEY_AF_ON = "XF86Launch9";
    private static final String KEY_LIGHT = "XF86TouchpadToggle";
    private static final String KEY_MF_ZOOM = "XF86TouchpadOff";
    private static final String KEY_WIFI = "XF86Mail";

    private static final String KEY_JOG1_CW = "XF86ScrollUp";
    private static final String KEY_JOG1_CCW = "XF86ScrollDown";
    private static final String KEY_JOG2_CW = "parenleft";
    private static final String KEY_JOG2_CCW = "parenright";
    private static final String KEY_JOG_CW = "XF86AudioNext";
    private static final String KEY_JOG_CCW = "XF86AudioPrev";

    private static final String KEY_MODE_SCENE = "XF86Send";
    private static final String KEY_MODE_SMART = "XF86Reply";
    private static final String KEY_MODE_P = "XF86MailForward";
    private static final String KEY_MODE_A = "XF86Save";
    private static final String KEY_MODE_S = "XF86Documents";
    private static final String KEY_MODE_M = "XF86Battery";
    private static final String KEY_MODE_CUSTOM1 = "XF86WLAN";
    private static final String KEY_MODE_CUSTOM2 = "XF86Bluetooth";
    //private static final String KEY_MODE_SAS = "";

    private static final String KEY_WHEEL_CW = "Left";
    private static final String KEY_WHEEL_CCW = "Right";

    private static final String KEY_DRIVE_SINGLE = "XF86Search";
    private static final String KEY_DRIVE_CONTI_N = "XF86Go";
    private static final String KEY_DRIVE_CONTI_H = "XF86Finance";
    private static final String KEY_DRIVE_TIMER = "XF86Game";
    private static final String KEY_DRIVE_BRACKET = "XF86Shop";

    private void runCommand(String command) {
        if (mCommandExecutor != null) {
            mCommandExecutor.execute(command);
        }
    }

    public void onButtonClick(View v) {
        String key = "";
        switch (v.getId()) {
            case R.id.keyWifi:
                key = KEY_WIFI;
                break;
            case R.id.keyAEL:
                key = KEY_AEL;
                break;
            case R.id.keyRec:
                key = KEY_REC;
                break;

            case R.id.keyMenu:
                key = KEY_MENU;
                break;
            case R.id.keyUp:
                key = KEY_UP;
                break;
            case R.id.keyFn:
                key = KEY_FN;
                break;
            case R.id.keyLeft:
                key = KEY_LEFT;
                break;
            case R.id.keyOK:
                key = KEY_OK;
                break;
            case R.id.keyRight:
                key = KEY_RIGHT;
                break;
            case R.id.keyPB:
                key = KEY_PB;
                break;
            case R.id.keyDown:
                key = KEY_DOWN;
                break;
            case R.id.keyDel:
                key = KEY_DEL;
                break;
        }
        if (!key.equals("")) {
            runCommand("/opt/usr/scripts/chroot.sh xdotool key " + key);
        }
    }

    private class TextDrawable extends Drawable {
        private static final int DEFAULT_COLOR = Color.WHITE;
        private static final int DEFAULT_TEXTSIZE = 15;
        private Paint mPaint;
        private CharSequence mText;
        private int mIntrinsicWidth;
        private int mIntrinsicHeight;

        public TextDrawable(Resources res, CharSequence text) {
            mText = text;
            mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mPaint.setColor(DEFAULT_COLOR);
            mPaint.setTextAlign(Paint.Align.CENTER);
            float textSize = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP,
                    DEFAULT_TEXTSIZE, res.getDisplayMetrics());
            mPaint.setTextSize(textSize);
            mIntrinsicWidth = (int) (mPaint.measureText(mText, 0, mText.length()) + .5);
            mIntrinsicHeight = mPaint.getFontMetricsInt(null);
        }
        @Override
        public void draw(Canvas canvas) {
            Rect bounds = getBounds();
            canvas.drawText(mText, 0, mText.length(),
                    bounds.centerX(), bounds.centerY(), mPaint);
        }
        @Override
        public int getOpacity() {
            return mPaint.getAlpha();
        }
        @Override
        public int getIntrinsicWidth() {
            return mIntrinsicWidth;
        }
        @Override
        public int getIntrinsicHeight() {
            return mIntrinsicHeight;
        }
        @Override
        public void setAlpha(int alpha) {
            mPaint.setAlpha(alpha);
        }
        @Override
        public void setColorFilter(ColorFilter filter) {
            mPaint.setColorFilter(filter);
        }
    }

    private class ModeWheelAdapter implements WheelAdapter {
        private class Mode {
            public String mMode;
            public TextDrawable mDrawable;
            public String mKey;
            public Mode(String mode, String key) {
                mMode = mode;
                mDrawable = new TextDrawable(res, mode);
                mKey = key;
            }
            public String getMode() {
                return mMode;
            }
            public Drawable getDrawable() {
                return mDrawable;
            }
            public String getKey() {
                return mKey;
            }
        }

        private Resources res = getResources();
        Mode[] mModes = {
                new Mode("c", KEY_MODE_CUSTOM1),
                new Mode("m", KEY_MODE_M),
                new Mode("s", KEY_MODE_S),
                new Mode("a", KEY_MODE_A),
                new Mode("p", KEY_MODE_P),
                new Mode("auto", KEY_MODE_SMART),
                new Mode("scn", KEY_MODE_SCENE),
                //new Mode("SAS", KEY_MODE_SAS), // FIXME: find KEY_MODE_SAS
        };
        private int mSelectedPosition;

        @Override
        public Drawable getDrawable(int position) {
            return mModes[position].getDrawable();
        }

        @Override
        public int getCount() {
            return mModes.length;
        }

        public void setSelectedPosition(int position) {
            mSelectedPosition = position;
        }

        public int getSelectedPosition() {
            return mSelectedPosition;
        }

        public String getKeyCodeOfSelectedPosition() {
            return mModes[mSelectedPosition].getKey();
        }
        public String getModeOfSelectedPosition() {
            return mModes[mSelectedPosition].getMode();
        }
    }

    private class JogWheelAdapter implements WheelAdapter {
        private Drawable mDrawable = new TextDrawable(getResources(), "I");

        @Override
        public Drawable getDrawable(int position) {
            return mDrawable;
        }

        @Override
        public int getCount() {
            return 50;
        }
    }
}
