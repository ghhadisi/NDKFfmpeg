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

//重采样
#include "libswresample/swresample.h"
    JNIEXPORT void JNICALL
    Java_com_dbgs_ffmpegaudiolib_AudioPlayer_sound(JNIEnv *env, jobject instance, jstring input_,
                                                   jstring output_) {
        const char *input = env->GetStringUTFChars(input_, 0);
        const char *output = env->GetStringUTFChars(output_, 0);

        // TODO
        av_register_all();
        AVFormatContext *pContext = avformat_alloc_context();
        if (avformat_open_input(&pContext,input,NULL,NULL) !=0){
            LOGI("%s","无法打开音频文件");
            return;
        }

        if (avformat_find_stream_info(pContext,NULL) != 0 ){
            LOGI("%s","无法获取输入文件信息");
            return;
        }
        //获取音频流索引位置
        int audio_stream_idx = -1;
        for (int i=0,size=  pContext->nb_streams; i<size;i++){
            if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
                audio_stream_idx = i;
                break;
            }
        }
        if (-1 == audio_stream_idx){
            return;
        }
        //获取解码器
        AVCodecContext * codecContext = pContext->streams[audio_stream_idx]->codec;
        AVCodec *  codec   = avcodec_find_decoder(codecContext->codec_id);

        if (avcodec_open2(codecContext,codec,NULL) <0){
            LOGI("%s","无法打开解码器");
            return;
        }
        //压缩数据
        AVPacket * packet = (AVPacket * )av_malloc(sizeof(AVPacket));
        //解压缩数据
        AVFrame * frame = av_frame_alloc();
        //frame->16bit 44100 PCM 统一音频采样格式与采样率
        SwrContext * swrContext = swr_alloc();
        //    音频格式  重采样设置参数
        AVSampleFormat in_sample = codecContext->sample_fmt;
        //    输出采样格式
        AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;
        // 输入采样率
        int in_sample_rate = codecContext->sample_rate;
        //    输出采样
        int out_sample_rate = 44100;

        //    输入声道布局
        uint64_t  in_ch_layout = codecContext->channel_layout;

        //    输出声道布局
        uint64_t out_ch_layout =  AV_CH_LAYOUT_STEREO;//双通道

        /**
    * struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
     int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
     int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
     int log_offset, void *log_ctx);
    */
        swr_alloc_set_opts(swrContext, out_ch_layout, out_sample,out_sample_rate
        ,in_ch_layout,in_sample,in_sample_rate,
        0,NULL);
        swr_init(swrContext);

        int got_frame=0;
        int ret;

        int out_channerl_nb = av_get_channel_layout_nb_channels(out_ch_layout);
        LOGE("声道数量%d ",out_channerl_nb);

        int count=0;
//    设置音频缓冲区间 16bit   44100  PCM数据
        uint8_t  * out_buffer =( uint8_t  *) av_malloc(44100 * 2);
        FILE * fp_pcm = fopen(output,"wb");
        while (av_read_frame(pContext,packet) >= 0){
            ret = avcodec_decode_audio4(codecContext,frame,&got_frame,packet);
            if (ret < 0) {
                LOGE("解码完成");
            }
            //        解码一帧
            if (got_frame > 0) {
                /**
                  * int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
                                     const uint8_t **in , int in_count);
                  */
                swr_convert(swrContext,&out_buffer,2*44100, (const uint8_t **)frame->data,frame->nb_samples);
                /**
                * int av_samples_get_buffer_size(int *linesize, int nb_channels, int nb_samples,
                                  enum AVSampleFormat sample_fmt, int align);
                */
                int out_buffer_size=  av_samples_get_buffer_size(NULL,out_channerl_nb,frame->nb_samples,out_sample,1);
                fwrite(out_buffer,1,out_buffer_size,fp_pcm);

            }
            av_free_packet(packet);
        }
        av_frame_free(&frame);
        av_free(out_buffer);

        swr_free(&swrContext);
        avcodec_close(codecContext);
        avformat_close_input(&pContext);


        env->ReleaseStringUTFChars(input_, input);
        env->ReleaseStringUTFChars(output_, output);
    }
}
