//
// Created by Administrator on 2017/10/3.
//

#include <jni.h>
#include <string>
#include <android/log.h>



#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);



extern "C" {

//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libswscale/swscale.h"

#include <android/native_window_jni.h>
#include <unistd.h>

JNIEXPORT void JNICALL
    Java_com_dbgs_ffmpeglib2_VideoView_render(JNIEnv *env, jobject instance, jstring input_,
                                              jobject surface) {
        const char *input = env->GetStringUTFChars(input_, 0);
        //    注册各大组件
        av_register_all();
        AVFormatContext *pContext = avformat_alloc_context();
    //第四个参数是 可以传一个 字典   是一个入参出参对象
        if (avformat_open_input(&pContext,input,NULL,NULL) != 0){// <0
            LOGE("打开失败");
            return;
        }
        //3.获取视频信息
        if (avformat_find_stream_info(pContext,NULL) < 0){
            LOGE("%s","获取视频信息失败");
            return;
        }

        int vidio_stream_idx=-1;
        for (int i=0,size = pContext->nb_streams;i<size;i++){
            if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
                LOGE("  找到视频id %d", pContext->streams[i]->codec->codec_type);
                vidio_stream_idx = i;
                break;
            }
        }

    //    获取视频编解码器
        AVCodecContext *codecContext = pContext->streams[vidio_stream_idx]->codec;
        LOGE("获取视频编码器上下文 %p  ",codecContext);
    //    加密的用不了
        AVCodec * codec = avcodec_find_decoder(codecContext->codec_id);
        LOGE("获取视频编码 %p",codec);
        if (avcodec_open2(codecContext,codec,NULL) <0){
            LOGE("avcodec_open2打开失败");
            return;
        }

        AVPacket * packet = (AVPacket *)av_malloc(sizeof(AVPacket));
        av_init_packet(packet);
    //    像素数据
        AVFrame *frame = av_frame_alloc();
        AVFrame * rgbFrame = av_frame_alloc();

    //   给缓冲区分配内存
    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
        uint8_t  * out_buffer = (uint8_t  *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA,codecContext->width,codecContext->height));
    LOGE("宽  %d,  高  %d  ",codecContext->width,codecContext->height);
    //设置yuvFrame的缓冲区，像素格式
     int re = avpicture_fill((AVPicture *)rgbFrame, out_buffer, AV_PIX_FMT_RGBA, codecContext->width, codecContext->height);
    LOGE("申请内存%d   ",re);
    //    输出需要改变
    int length=0;
    int got_frame;
//    输出文件
    int frameCount=0;
    SwsContext * swsContext = sws_getContext(codecContext->width,codecContext->height,codecContext->pix_fmt,
                                             codecContext->width,codecContext->height,AV_PIX_FMT_RGBA,
                                             SWS_BICUBIC,NULL,NULL,NULL
    );

    ANativeWindow * nativeWindow = ANativeWindow_fromSurface(env, surface);
//    视频缓冲区
    ANativeWindow_Buffer windowBuffer;

    while (av_read_frame(pContext,packet) >= 0){
        //        节封装
//        根据frame 进行原生绘制    bitmap  window
        if (packet->stream_index == vidio_stream_idx){
            length =avcodec_decode_video2(codecContext,frame,&got_frame,packet);
            LOGE(" 获得长度   %d ", length);

            //非零   正在解码
            if (got_frame) {
                //            绘制之前   配置一些信息  比如宽高   格式
                ANativeWindow_setBuffersGeometry(nativeWindow,codecContext->width,codecContext->height,
                                                 WINDOW_FORMAT_RGBA_8888);

//                绘制
                ANativeWindow_lock(nativeWindow,&windowBuffer,NULL);
                //     h 264   ----yuv          RGBA
                LOGI("解码%d帧",frameCount++);
                //转为指定的YUV420P
                sws_scale(swsContext,(const uint8_t *const *) frame->data,frame->linesize,
                0,frame->height,rgbFrame->data,
                          rgbFrame->linesize );
                //rgb_frame是有画面数据
                uint8_t *dst= (uint8_t *) windowBuffer.bits;
                //拿到一行有多少个字节 RGBA
                int destStride=windowBuffer.stride*4;
                //像素数据的首地址
                uint8_t * src= (uint8_t *) rgbFrame->data[0];
                //实际内存一行数量
                int srcStride = rgbFrame->linesize[0];



                for (int i = 0; i < codecContext->height; ++i) {
//                memcpy(void *dest, const void *src, size_t n)
                    memcpy(dst + i * destStride,  src + i * srcStride, srcStride);
                }

                ANativeWindow_unlockAndPost(nativeWindow);
                usleep(1000 * 16);

            }
        }
        av_free_packet(packet);

    }
    av_frame_free(&frame);
        av_frame_free(&rgbFrame);
        avcodec_close(codecContext);
        avformat_free_context(pContext);


    env->ReleaseStringUTFChars(input_, input);
    }


}
