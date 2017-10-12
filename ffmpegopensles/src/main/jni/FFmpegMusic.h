//
// Created by david on 2017/9/25.
//

#ifndef FFMPEGOPENSL_FFMPEGMUSIC_H
#define FFMPEGOPENSL_FFMPEGMUSIC_H

#endif //FFMPEGOPENSL_FFMPEGMUSIC_H
extern "C" {
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
//像素处理
#include "libswscale/swscale.h"
#include <android/native_window_jni.h>
#include <unistd.h>
#include <android/log.h>
}
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);
int createFFmpeg(int *rate,int *channel );

void getPCM(void **outBuffer, size_t *size);

void realase();