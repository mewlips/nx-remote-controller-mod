package com.mewlips.nxremote;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.provider.Settings;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.lukedeighton.wheelview.WheelView;
import com.lukedeighton.wheelview.adapter.WheelAdapter;

import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import static com.mewlips.nxremote.Configurations.FRAME_HEIGHT;
import static com.mewlips.nxremote.Configurations.FRAME_WIDTH;
import static com.mewlips.nxremote.Configurations.GET_MOV_SIZE_COMMAND_NX500;
import static com.mewlips.nxremote.Configurations.INJECT_INPUT_COMMAND;
import static com.mewlips.nxremote.Configurations.LCD_OFF_COMMAND;
import static com.mewlips.nxremote.Configurations.LCD_ON_COMMAND;
import static com.mewlips.nxremote.Configurations.LCD_VIDEO_COMMAND;
import static com.mewlips.nxremote.Configurations.MOD_GUI_COMMAND_NX500;
import static com.mewlips.nxremote.Configurations.VIDEO_SIZE_FHD_HD;
import static com.mewlips.nxremote.Configurations.VIDEO_SIZE_NORMAL;
import static com.mewlips.nxremote.Configurations.VIDEO_SIZE_UHD;
import static com.mewlips.nxremote.Configurations.VIDEO_SIZE_VGA;
import static com.mewlips.nxremote.NXKeys.KEY_AEL;
import static com.mewlips.nxremote.NXKeys.KEY_DEL;
import static com.mewlips.nxremote.NXKeys.KEY_DOWN;
import static com.mewlips.nxremote.NXKeys.KEY_EV;
import static com.mewlips.nxremote.NXKeys.KEY_FN;
import static com.mewlips.nxremote.NXKeys.KEY_JOG1_CCW;
import static com.mewlips.nxremote.NXKeys.KEY_JOG1_CW;
import static com.mewlips.nxremote.NXKeys.KEY_JOG_CCW;
import static com.mewlips.nxremote.NXKeys.KEY_JOG_CW;
import static com.mewlips.nxremote.NXKeys.KEY_LEFT;
import static com.mewlips.nxremote.NXKeys.KEY_MENU;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_A;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_A_GET;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_CUSTOM1;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_CUSTOM1_GET;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_CUSTOM2;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_M;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_M_GET;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_P;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_P_GET;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_S;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_SAS;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_SAS_GET;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_SCENE;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_SCENE_GET;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_SMART;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_SMART_GET;
import static com.mewlips.nxremote.NXKeys.KEY_MODE_S_GET;
import static com.mewlips.nxremote.NXKeys.KEY_OK;
import static com.mewlips.nxremote.NXKeys.KEY_PB;
import static com.mewlips.nxremote.NXKeys.KEY_REC;
import static com.mewlips.nxremote.NXKeys.KEY_RIGHT;
import static com.mewlips.nxremote.NXKeys.KEY_S1;
import static com.mewlips.nxremote.NXKeys.KEY_S2;
import static com.mewlips.nxremote.NXKeys.KEY_UP;

public class MainActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {

    private static final String TAG = "MainActivity";
    private static final boolean DEBUG = true;

    private NXCameraInfo mCameraInfo;
    private NotifyReceiver mNotifyReceiver;
    private VideoPlayer mVideoPlayer;
    private XWinViewer mXWinViewer;
    private CommandExecutor mCommandExecutor;
    private DiscoveryPacketReceiver mDiscoveryPacketReceiver;

    private ImageView mImageViewVideo;
    private ImageView mImageViewXWin;
    private ModeWheelAdapter mModeWheelAdapter;
    private WheelView mWheelViewMode;
    private JogWheelAdapter mJogWheelAdapterJog1;
    private WheelView mWheelViewJog1;
    private JogWheelAdapter mJogWheelAdapterJog2;
    private WheelView mWheelViewJog2;
    private ImageView mShutterButton;
    private TextView mButtonEv;
    private TextView mTextViewStatus;

    private boolean mCameraVideoEnabled = true;
    private boolean mCameraXWinEnabled = true;
    private boolean mMobileVideoEnabled = true;
    private boolean mMobileXwinEnabled = true;
    private boolean mVideoDrawEnabled = true;

    protected String getWifiIpAddress() {
        WifiManager wifiMan = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInf = wifiMan.getConnectionInfo();
        int ipAddress = wifiInf.getIpAddress();
        return String.format(Locale.ENGLISH, "%d.%d.%d.%d",
                (ipAddress & 0xff),(ipAddress >> 8 & 0xff),
                (ipAddress >> 16 & 0xff),(ipAddress >> 24 & 0xff));
    }

    protected void onRecordStarted() {
        mVideoDrawEnabled = false;
        runCommand("vfps=1");
        runCommand("xfps=1");
        runCommand(GET_MOV_SIZE_COMMAND_NX500); // NX500
    }

    protected void onRecordStopped() {
        mVideoDrawEnabled = false;
        runCommand("vfps=4");
        runCommand("xfps=4");
        mVideoPlayer.setVideoSize(VIDEO_SIZE_NORMAL);
        setVideoMargin();
    }

    protected void setVideoMargin() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int width = mImageViewXWin.getWidth();
                int height = mImageViewXWin.getHeight();
                float wr = (float) width / 720f;
                float hr = (float) height / 480f;
                FrameLayout.LayoutParams layoutParams = (FrameLayout.LayoutParams) mImageViewVideo.getLayoutParams();
                switch (mVideoPlayer.getVideoSize()) {
                    case VIDEO_SIZE_FHD_HD:
                        layoutParams.setMargins(0, (int) (38 * hr), 0, (int) (38 * hr));
                        break;
                    case VIDEO_SIZE_UHD:
                        layoutParams.setMargins(0, (int) (50 * hr), 0, (int) (50 * hr));
                        break;
                    case VIDEO_SIZE_VGA:
                        layoutParams.setMargins((int) (40 * wr), 0, (int) (40 * wr), 0);
                        break;
                    default:
                        layoutParams.setMargins(0, 0, 0, 0);
                        break;
                }
                mImageViewVideo.setLayoutParams(layoutParams);
            }
        });

        Timer timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                mVideoDrawEnabled = true;
            }
        }, 1500);
    }

    private void setConnectionStatus(final boolean onConnected) {
        Log.d(TAG, "setConnectionStatus(), onConnected = " + onConnected);
//        runOnUiThread(new Runnable() {
//            @Override
//            public void run() {
//                if (onConnected) {
//                    mTextViewStatus.setText("Camera connected.");
//                    mTextViewStatus.append("\nIP: " + mCameraInfo.getIpAddress());
//                    Timer timer = new Timer();
//                    timer.schedule(new TimerTask() {
//                        @Override
//                        public void run() {
//                            runOnUiThread(new Runnable() {
//                                @Override
//                                public void run() {
//                                    mTextViewStatus.setText("");
//                                }
//                            });
//                        }
//                    }, 3000);
//                } else {
//                    mTextViewStatus.setText("Camera disconnected.");
//                }
//            }
//        });
    }

    protected void startDiscovery() {
        if (mDiscoveryPacketReceiver == null) {
            DiscoveryPacketReceiver.DiscoveryListener discoveryListener
                    = new DiscoveryPacketReceiver.DiscoveryListener() {
                @Override
                public void onFound(String version, String model, String ipAddress) {
                    stopDiscovery();
                    disconnectFromCameraDaemon();
                    mCameraInfo = new NXCameraInfo(version, model, ipAddress);
                    connectToCameraDaemon();
                }
            };
            mDiscoveryPacketReceiver = new DiscoveryPacketReceiver(discoveryListener);
            mDiscoveryPacketReceiver.start();
        }
    }

    private void stopDiscovery() {
        if (mDiscoveryPacketReceiver != null) {
            mDiscoveryPacketReceiver.closeSocket();
            mDiscoveryPacketReceiver = null;
        }
    }

    private void connectToCameraDaemon() {
        Log.d(TAG, "connectToCameraDaemon()");

        if (mCameraVideoEnabled && mCameraXWinEnabled) {
            runCommand(LCD_ON_COMMAND);
        } else if (mCameraVideoEnabled && !mCameraXWinEnabled) {
            runCommand(LCD_VIDEO_COMMAND);
        } else {
            runCommand(LCD_OFF_COMMAND);
        }

        if (mMobileVideoEnabled) {
            startVideoPlayer();
            setVideoVisibility(true);
        } else {
            if (mVideoPlayer != null) {
                mVideoPlayer.closeSocket();
            }
            setVideoVisibility(false);
        }
        if (mMobileXwinEnabled) {
            startXWinViewer();
            setXWinVisibility(true);
        } else {
            if (mXWinViewer != null) {
                mXWinViewer.closeSocket();
            }
            setXWinVisibility(false);
        }

        startExecutor();
        startNotifyReceiver();

        setConnectionStatus(true);
        if (mCameraInfo.isRecording()) {
            runCommand("vfps=1");
            runCommand("xfps=1");
        } else {
            runCommand("vfps=4");
            runCommand("xfps=4");
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

//        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
//        fab.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
//                        .setAction("Action", null).show();
//            }
//        });

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.setDrawerListener(toggle);
        toggle.syncState();

        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);

        View.OnTouchListener onTouchListener = new View.OnTouchListener() {
            private static final int SKIP_MOUSE_MOVE_COUNT = 4;
            private int mSkipCount;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction() & MotionEvent.ACTION_MASK;
                int x = (int) (event.getX() * (float)FRAME_WIDTH / (float)v.getWidth());
                int y = (int) (event.getY() * (float)FRAME_HEIGHT / (float)v.getHeight());
                String command = INJECT_INPUT_COMMAND;

                Log.d(TAG, "action = " + action + ", x = " + x + ", y = " + y);

                switch (action) {
                    case MotionEvent.ACTION_DOWN:
                        mSkipCount = SKIP_MOUSE_MOVE_COUNT;
                        runCommand(command + "mousemove " + x + " " + y);
                        runCommand(command + "mousedown 1");
                        return true;
                    case MotionEvent.ACTION_MOVE:
                        mSkipCount--;
                        if (mSkipCount == 0) {
                            runCommand(command + "mousemove " + x + " " + y);
                            mSkipCount = SKIP_MOUSE_MOVE_COUNT;
                        }
                        return true;
                    case MotionEvent.ACTION_UP:
                        runCommand(command + "mousemove " + x + " " + y);
                        runCommand(command + "mouseup 1");
                        break;
                }
                return false;
            }
        };

        int[] intArray = new int[FRAME_WIDTH * FRAME_HEIGHT];
        Bitmap bmp = Bitmap.createBitmap(intArray, FRAME_WIDTH, FRAME_HEIGHT, Bitmap.Config.ARGB_8888);

        findViewById(R.id.layoutLcd).setOnTouchListener(onTouchListener);

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
                    keyClick(keyCode);
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
                keyClick(key);
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
                keyClick(key);
                mPrevPosition = position;
            }
        });

        mShutterButton = (ImageView) findViewById(R.id.shutterButton);
        mShutterButton.setOnTouchListener(new View.OnTouchListener() {
            private static final int SKIP_MOVE_COUNT = 5;
            private int mSkipCount;

            private float mPrevY;
            private int mPrevHeight;
            private boolean mS1Downed;
            private boolean mS2Downed;
             @Override
             public boolean onTouch(View v, MotionEvent event) {
                 float currY;
                 int currHeight;
                 RelativeLayout.LayoutParams layoutParams;

                 int action = event.getAction() & MotionEvent.ACTION_MASK;
                 switch (action) {
                     case MotionEvent.ACTION_DOWN:
                         mPrevY = event.getY();
                         mPrevHeight = mShutterButton.getHeight();
                         keyDown(KEY_S1);
                         mS1Downed = true;
                         Log.d(TAG, "S1 down");
                         mSkipCount = SKIP_MOVE_COUNT;
                         break;
                     case MotionEvent.ACTION_MOVE:
                         currY = event.getY();
                         currHeight = mShutterButton.getHeight();
                         if (mPrevY < currY) {
                             if ((float)currHeight / (float)mPrevHeight > 0.9) {
                                 layoutParams = (RelativeLayout.LayoutParams) mShutterButton.getLayoutParams();
                                 layoutParams.setMargins(0, (int) (currY - mPrevY), 0, 0);
                                 mShutterButton.setLayoutParams(layoutParams);
                             }
                         }
                         if (mSkipCount == 0) {
                             if (currHeight < mPrevHeight) {
                                 if (!mS2Downed) {
                                     keyDown(KEY_S2);
                                     mS2Downed = true;
                                     Log.d(TAG, "S2 down");
                                 }
                             } else {
                                 if (mS2Downed) {
                                     keyUp(KEY_S2);
                                     mS2Downed = false;
                                     Log.d(TAG, "S2 up");
                                 }
                             }
                             mSkipCount = SKIP_MOVE_COUNT;
                         } else {
                             mSkipCount--;
                         }
                         break;
                     case MotionEvent.ACTION_UP:
                         layoutParams = (RelativeLayout.LayoutParams) mShutterButton.getLayoutParams();
                         layoutParams.setMargins(0, 0, 0, 0);
                         mShutterButton.setLayoutParams(layoutParams);
                         if (mS2Downed) {
                             keyUp(KEY_S2);
                             mS2Downed = false;
                             Log.d(TAG, "S2 up");
                         }
                         if (mS1Downed) {
                             keyUp(KEY_S1);
                             mS1Downed = false;
                             Log.d(TAG, "S1 up");
                         }
                         return false;
                     default:
                         return false;
                 }
                 return true;
             }
        });

        mButtonEv = (TextView) findViewById(R.id.keyEV);
        mButtonEv.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction() & MotionEvent.ACTION_MASK;
                switch (action) {
                    case MotionEvent.ACTION_DOWN:
                        runCommand(INJECT_INPUT_COMMAND + "keydown " + KEY_EV);
                        return true;
                    case MotionEvent.ACTION_UP:
                        runCommand(INJECT_INPUT_COMMAND + "keyup " + KEY_EV);
                        break;
                    default:
                        return true;
                }
                return false;
            }
        });

        mTextViewStatus = (TextView) findViewById(R.id.textViewStatus);
    }

    protected void startNotifyReceiver() {
        if (mNotifyReceiver == null) {
            mNotifyReceiver = new NotifyReceiver(mCameraInfo, this);
            mNotifyReceiver.start();
        }
    }

    protected void startVideoPlayer() {
        if (mVideoPlayer == null && mMobileVideoEnabled) {
            mVideoPlayer = new VideoPlayer(mCameraInfo, this);
            mVideoPlayer.start();
        }
    }

    protected void startXWinViewer() {
        if (mXWinViewer == null && mMobileXwinEnabled) {
            mXWinViewer = new XWinViewer(mCameraInfo, this);
            mXWinViewer.start();
        }
    }

    protected void startExecutor() {
        if (mCommandExecutor == null) {
            mCommandExecutor = new CommandExecutor(mCameraInfo, this);
            new Thread(mCommandExecutor).start();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        startDiscovery();
    }

    @Override
    protected void onPause() {
        super.onPause();

        stopDiscovery();
        disconnectFromCameraDaemon();
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
            Toast.makeText(this, "Not implemented yet.", Toast.LENGTH_SHORT).show();
            return true;
        } else if (id == R.id.action_open_mod) {
            runCommand(MOD_GUI_COMMAND_NX500);
        } else if (id == R.id.action_wifi_settings) {
            startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));
        } else if (id == R.id.action_hotspot_settings) {
            final Intent intent = new Intent(Intent.ACTION_MAIN, null);
            intent.addCategory(Intent.CATEGORY_LAUNCHER);
            final ComponentName cn = new ComponentName("com.android.settings", "com.android.settings.TetherSettings");
            intent.setComponent(cn);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }

        return super.onOptionsItemSelected(item);
    }

    @SuppressWarnings("StatementWithEmptyBody")
    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        int id = item.getItemId();

        if (id == R.id.nav_lv_osd_lv_osd) {
            mCameraVideoEnabled = true;
            mCameraXWinEnabled = true;
            mMobileVideoEnabled = true;
            mMobileXwinEnabled = true;
        } else if (id == R.id.nav_lv_osd_lv) {
            mCameraVideoEnabled = true;
            mCameraXWinEnabled = true;
            mMobileVideoEnabled = true;
            mMobileXwinEnabled = false;
            closeXWinViewer();
        } else if (id == R.id.nav_lv_osd_osd) {
            mCameraVideoEnabled = true;
            mCameraXWinEnabled = true;
            mMobileVideoEnabled = false;
            mMobileXwinEnabled = true;
            closeVideoPlayer();
        } else if (id == R.id.nav_lv_osd_off) {
            mCameraVideoEnabled = true;
            mCameraXWinEnabled = true;
            mMobileVideoEnabled = false;
            mMobileXwinEnabled = false;
            closeVideoPlayer();
            closeXWinViewer();
        } else if (id == R.id.nav_lv_lv) {
            mCameraVideoEnabled = true;
            mCameraXWinEnabled = false;
            mMobileVideoEnabled = true;
            mMobileXwinEnabled = false;
        } else if (id == R.id.nav_lv_off) {
            mCameraVideoEnabled = true;
            mCameraXWinEnabled = false;
            mMobileVideoEnabled = false;
            mMobileXwinEnabled = false;
            closeVideoPlayer();
        } else if (id == R.id.nav_off_lv) {
            mCameraVideoEnabled = false;
            mCameraXWinEnabled = false;
            mMobileVideoEnabled = true;
            mMobileXwinEnabled = false;
            closeXWinViewer();
        } else if (id == R.id.nav_off_off) {
            mCameraVideoEnabled = false;
            mCameraXWinEnabled = false;
            mMobileVideoEnabled = false;
            mMobileXwinEnabled = false;
            closeVideoPlayer();
            closeXWinViewer();
        }

        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                connectToCameraDaemon();
            }
        });

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    protected void closeNotifyReceiver() {
        if (mNotifyReceiver != null) {
            mNotifyReceiver.closeSocket();
            mNotifyReceiver = null;
        }
    }

    protected void closeVideoPlayer() {
        if (mVideoPlayer != null) {
            mVideoPlayer.closeSocket();
            mVideoPlayer = null;
            setVideoVisibility(false);
        }
    }

    protected void closeXWinViewer() {
        if (mXWinViewer != null) {
            mXWinViewer.closeSocket();
            mXWinViewer = null;
            setXWinVisibility(false);
        }
    }

    protected void closeCommandExecutor() {
        if (mCommandExecutor != null) {
            mCommandExecutor.closeSocket();
            mCommandExecutor = null;
        }
    }

    protected void disconnectFromCameraDaemon() {
        Log.d(TAG, "disconnectFromCameraDaemon()");

        closeNotifyReceiver();
        closeVideoPlayer();
        closeXWinViewer();
        closeCommandExecutor();

//        mCameraInfo = null;

        setConnectionStatus(false);
    }

    private void runCommand(String command) {
        if (mCommandExecutor != null) {
            mCommandExecutor.execute(command);
        }
    }

    protected void setVideoSize(int videoSize) {
        if (mVideoPlayer != null) {
            mVideoPlayer.setVideoSize(videoSize);
        }
    }

    public void onButtonClick(View v) {
        String key = "";
        switch (v.getId()) {
            case R.id.keyAEL:
                key = KEY_AEL;
                break;
            case R.id.keyEV:
                key = KEY_EV;
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
            keyClick(key);
        }
    }

    private void keyDown(String key) {
        runCommand(INJECT_INPUT_COMMAND + "keydown " + key);
    }

    private void keyUp(String key) {
        runCommand(INJECT_INPUT_COMMAND + "keyup " + key);
    }

    private void keyClick(String key) {
        runCommand(INJECT_INPUT_COMMAND + "key " + key);
    }

    protected void setVideoBitmap(final Bitmap bitmap) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mVideoDrawEnabled) {
                    mImageViewVideo.setImageBitmap(bitmap);
                }
            }
        });
    }

    protected void setXWinBitmap(final Bitmap bitmap) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mImageViewXWin.setImageBitmap(bitmap);
            }
        });
    }
    protected void setXWinVisibility(final boolean visible) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mImageViewXWin.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
            }
        });
    }

    protected void setVideoVisibility(final boolean visible) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mImageViewVideo.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
            }
        });
    }
    protected void setButtonBackground(final String key, final boolean keyDown) {
        Log.d(TAG, "key = " + key + " (down = " + keyDown + ")");

        String mode = "";
        switch (key) {
            case KEY_MODE_CUSTOM1_GET:
                mode = "C1";
                break;
            case KEY_MODE_M_GET:
                mode = "M";
                break;
            case KEY_MODE_S_GET:
                mode = "S";
                break;
            case KEY_MODE_A_GET:
                mode = "A";
                break;
            case KEY_MODE_P_GET:
                mode = "P";
                break;
            case KEY_MODE_SMART_GET:
                mode = "AUTO";
                break;
            case KEY_MODE_SCENE_GET:
                mode = "SCN";
                break;
            case KEY_MODE_SAS_GET:
                mode = "SAS";
                break;
            case KEY_MODE_CUSTOM2:
                mode = "C2";
                break;
        }
        if (keyDown) {
            int position = mModeWheelAdapter.getPositionByMode(mode);
            if (position != -1) {
                mModeWheelAdapter.setSelectedPosition(position);
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mWheelViewMode.setSelected(mModeWheelAdapter.getSelectedPosition());
                    }
                });
            }
        }

        TextView textView = null;
        switch (key) {
            case KEY_MENU:
                textView = (TextView) findViewById(R.id.keyMenu);
                break;
            case KEY_UP:
                textView = (TextView) findViewById(R.id.keyUp);
                break;
            case KEY_FN:
                textView = (TextView) findViewById(R.id.keyFn);
                break;
            case KEY_LEFT:
                textView = (TextView) findViewById(R.id.keyLeft);
                break;
            case KEY_OK:
                textView = (TextView) findViewById(R.id.keyOK);
                break;
            case KEY_RIGHT:
                textView = (TextView) findViewById(R.id.keyRight);
                break;
            case KEY_PB:
                textView = (TextView) findViewById(R.id.keyPB);
                break;
            case KEY_DOWN:
                textView = (TextView) findViewById(R.id.keyDown);
                break;
            case KEY_DEL:
                textView = (TextView) findViewById(R.id.keyDel);
                break;
        }
        if (textView != null) {
            final TextView finalTextView = textView;
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (keyDown) {
                        finalTextView.setBackgroundResource(R.drawable.rectangle_pressed);
                    } else {
                        finalTextView.setBackgroundResource(R.drawable.rectangle);
                    }
                }
            });
        }

        textView = null;
        switch (key) {
            case KEY_AEL:
                textView = (TextView) findViewById(R.id.keyAEL);
                break;
            case KEY_EV:
                textView = (TextView) findViewById(R.id.keyEV);
                break;
            case KEY_REC:
                textView = (TextView) findViewById(R.id.keyRec);
                break;
        }
        if (textView != null) {
            final TextView finalTextView = textView;
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    finalTextView.setBackgroundResource(keyDown ?
                            R.drawable.circle_pressed : R.drawable.circle);
                    if (key.equals(KEY_REC)) {
                        finalTextView.setTextColor(getResources().getColor(keyDown
                                ? R.color.colorRecButtonTextNormal
                                : R.color.colorRecButtonTextNormal));
                    }
                }
            });
        }
        if (key.equals(KEY_S1)) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    ImageView imageView = (ImageView) findViewById(R.id.shutterButton);
                    if (imageView != null) {
                        imageView.setBackgroundResource(keyDown ?
                                R.drawable.circle_shutter_s1 : R.drawable.circle_shutter);
                    }
                }
            });
        } else if (key.equals(KEY_S2)) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    ImageView imageView = (ImageView) findViewById(R.id.shutterButton);
                    if (imageView != null) {
                        imageView.setBackgroundResource(keyDown ?
                                R.drawable.circle_shutter_s2 : R.drawable.circle_shutter_s1);
                    }
                }
            });
        }
    }

    private class TextDrawable extends Drawable {
        private static final int DEFAULT_COLOR = Color.WHITE;
        private static final int DEFAULT_TEXTSIZE = 14;
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
                new Mode("C1", KEY_MODE_CUSTOM1),
                new Mode("M", KEY_MODE_M),
                new Mode("S", KEY_MODE_S),
                new Mode("A", KEY_MODE_A),
                new Mode("P", KEY_MODE_P),
                new Mode("AUTO", KEY_MODE_SMART),
                new Mode("SCN", KEY_MODE_SCENE),
                new Mode("SAS", KEY_MODE_SAS),
                new Mode("C2", KEY_MODE_CUSTOM2),
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

        public int getPositionByMode(String mode) {
            for (int i = 0; i < mModes.length; i++) {
                if (mModes[i].getMode().equals(mode)) {
                    return i;
                }
            }
            return -1;
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
