//
// Created by Administrator on 2017/10/7.
//
#include "FFmpegMusic.h"

AVFormatContext *pFormatCtx;
AVCodecContext *pCodecCtx;
AVCodec *pCodex;
AVFrame *frame;
SwrContext *swrContext;
AVPacket *packet;
int audio_stream_idx=-1;
uint8_t *out_buffer;

int createFFmpeg(int *rate,int *channel){
    // 本地音频文件
    char *input = "/sdcard/input.mp3";

    av_register_all();
    // 本地音频文件
    pFormatCtx = avformat_alloc_context();
    //第四个参数是 可以传一个 字典   是一个入参出参对象
    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0) {
        LOGE("%s","打开输入视频文件失败");
        return -1;
    }
    //3.获取视频信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        LOGE("%s","获取视频信息失败");
        return -1;
    }

    audio_stream_idx = -1;
    for (int  i=0,size = pFormatCtx->nb_streams;i<size;i++){
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx = i;
            break;
        }
    }
    if (-1 == audio_stream_idx){
        return -1;
    }

    // mp3的解码器
    //    获取音频编解码器
    pCodecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    LOGE("获取视频编码器上下文 %p  ",pCodecCtx);
    pCodex = avcodec_find_decoder(pCodecCtx->codec_id);
    LOGE("获取视频编码 %p",pCodex);
    if (avcodec_open2(pCodecCtx,pCodex,NULL) <0){
        return  -1;
    }

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    frame= av_frame_alloc();
    //    mp3  里面所包含的编码格式   转换成  pcm
    swrContext = swr_alloc();
    //    输出采样位数
    //输出的采样率必须与输入相同
    swr_alloc_set_opts(swrContext, AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,  pCodecCtx->sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0,
                       0);
    swr_init(swrContext);
    *rate = pCodecCtx->sample_rate;
    *channel = pCodecCtx->channels;
    out_buffer = (uint8_t *) av_malloc(44100 * 2);
    LOGE("初始化FFmpeg完毕");
    return 0;
}
void getPCM(void **outBuffer, size_t *size){
    int frameCount=0;
    int got_frame;
    int out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    while(av_read_frame(pFormatCtx,packet) >=0){
        if (packet->stream_index == audio_stream_idx){
            //            解码  mp3   编码格式frame----pcm   frame
             avcodec_decode_audio4(pCodecCtx,frame,&got_frame,packet);
            if (got_frame){
                LOGE("解码");
                /**
              * int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
                             const uint8_t **in , int in_count);
              */
                swr_convert(swrContext,&out_buffer,44100*2,(const uint8_t **)frame->data,frame->nb_samples);
                /**
              * int av_samples_get_buffer_size(int *linesize, int nb_channels, int nb_samples,
                                enum AVSampleFormat sample_fmt, int align);
              */
                int out_buffer_size = av_samples_get_buffer_size(NULL,out_channer_nb,frame->nb_samples,AV_SAMPLE_FMT_S16, 1);
                *outBuffer = out_buffer;
                *size=out_buffer_size;
                break;
            }
        }
    }
}

void realase(){
    av_free_packet(packet);
    av_frame_free(&frame);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}