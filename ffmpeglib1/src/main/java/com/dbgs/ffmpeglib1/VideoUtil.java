package com.dbgs.ffmpeglib1;

/**
 * Created by Administrator on 2017/10/3.
 */

public class VideoUtil {
    static {
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("video-lib");
    }

    public native void open(String inputStr, String outStr);

}
