package com.dbgs.ffmpegaudiolib;

/**
 * Created by Administrator on 2017/10/5.
 */

public class AudioPlayer {
    static {
        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("postproc-53");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("audio-lib");
    }

    public native void sound(String input,String output);

}
