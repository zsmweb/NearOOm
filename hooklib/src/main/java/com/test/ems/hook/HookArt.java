package com.test.ems.hook;

import android.os.Debug;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.IOException;
import java.lang.reflect.Method;

public class HookArt {
    private static final String TAG = "HookArt";
    public static String hprofPath = "/sdcard/test.hprof";

    static {
        try {
            System.loadLibrary("arthook");
        }catch (Error e) {
            e.printStackTrace();
        }
    }
    public static native int initHook();
    private static Handler H = null;
    private static Method dumpHprof = null;
    public static void doReport(int type) {
        if (H == null) {
            H = new Handler(Looper.getMainLooper());
        }
        H.post(new Runnable() {
            @Override
            public void run() {
                Log.e(TAG, "Near OOM! Trying to catch a hprof!");
                try {
                    Debug.dumpHprofData(hprofPath);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
