package com.dbgs.ffmpegopensles;

/**
 * Created by Administrator on 2017/10/7.
 */

public class AudioPlayer {
    static{
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avutil-54");
        System.loadLibrary("postproc-53");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("audio-lib");
    }
    public native void sound();
    public native void  stop();
}
