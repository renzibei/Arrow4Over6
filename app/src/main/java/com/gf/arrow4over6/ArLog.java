package com.gf.arrow4over6;

import android.util.Log;

public class ArLog {
    public static int d(String s) {
        return Log.d(Constants.DEBUG_TAG, s);
    }

    public static int e(String s) {
        return Log.e(Constants.ERROR_TAG, s);
    }

    public static int e(String s, Throwable t) {
        return Log.e(Constants.ERROR_TAG, s, t);
    }

    public static int v(String s) {
        return Log.v(Constants.VERBOSE_TAG, s);
    }

    public static int i(String s) {
        return Log.i(Constants.INFO_TAG, s);
    }

    public static int w(String s) {
        return Log.w(Constants.WARN_TAG, s);
    }
}

