//
// Created by Administrator on 2017/10/3.
//

#include <jni.h>
#include <string>
#include <android/log.h>
#include <assert.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#include "FFmpegMusic.h"

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




//// 创建Audio 结构体
SLObjectItf engineObject = NULL;
//音频引擎
SLEngineItf engineEngine;
SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
//混音器
SLObjectItf outputMixObject = NULL;
// buffer queue player interfaces
SLObjectItf bqPlayerObject = NULL;
//播放接口
SLPlayItf bqPlayerPlay;

SLVolumeItf bqPlayerVolume;
const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
size_t bufferSize;
void *buffer;

//缓冲器队列接口
SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
// 当喇叭播放完声音时回调此方法
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

JNIEXPORT void JNICALL
Java_com_dbgs_ffmpegopensles_AudioPlayer_sound(JNIEnv *env, jobject instance) {

    // TODO
    SLresult sLresult;
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // 获取SLEngine接口对象，后续的操作将使用这个对象
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    LOGE("地址 %p",engineEngine);
    //用音频引擎调用函数 创建混音器outputMixObject
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    // 实现混音器outputMixObject
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    // 获取混音器接口outputMixEnvironmentalReverb
    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == sLresult) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }

    int rate;
    int channels;
    createFFmpeg(&rate, &channels );
    LOGE(" 比特率%d  ,channels %d" ,rate,channels);
//  配置信息设置
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM pcm={SL_DATAFORMAT_PCM,2,SL_SAMPLINGRATE_44_1,SL_PCMSAMPLEFORMAT_FIXED_16
            ,SL_PCMSAMPLEFORMAT_FIXED_16,SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,SL_BYTEORDER_LITTLEENDIAN};
    //   新建一个数据源 将上述配置信息放到这个数据源中
//   新建一个数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};
    //    设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
//创建Recorder需要 RECORD_AUDIO 权限
//    SLInterfaceID slInterfaceID[2]={SL_IID_ANDROIDSIMPLEBUFFERQUEUE,SL_IID_ANDROIDCONFIGURATION};
//    SLboolean reqs[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    int reslut=SL_RESULT_SUCCESS==sLresult;
    //先讲这个
    sLresult = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &slDataSource, &audioSnk, 3,
                                                  ids, req);
    LOGE("初始化播放器%d  是否成功 %d   bqPlayerObject  %d",sLresult,reslut,bqPlayerObject);
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
//    得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    //    注册回调缓冲区 //获取缓冲队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueue);
    LOGE("获取缓冲区数据");
    //缓冲接口回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    //    获取音量接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
//    获取播放状态接口
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    bqPlayerCallback(bqPlayerBufferQueue, NULL);

}
// 当喇叭播放完声音时回调此方法
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){
    bufferSize = 0;
    //assert(NULL == context);
    getPCM(&buffer, &bufferSize);
    if (NULL != buffer && 0 != bufferSize) {
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer,
                                                 bufferSize);
        assert(SL_RESULT_SUCCESS == result);
        LOGE("david  bqPlayerCallback :%d", result);
    }
}

// shut down the native audio system
void shutdown()
{
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerVolume = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    // 释放FFmpeg解码器相关资源
    realase();
}


JNIEXPORT void JNICALL
Java_com_dbgs_ffmpegopensles_AudioPlayer_stop(JNIEnv *env, jobject instance) {

    // TODO
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerVolume = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    realase();
}
}



