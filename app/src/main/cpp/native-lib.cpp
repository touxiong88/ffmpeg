#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"sunhz",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"sunhz",FORMAT,##__VA_ARGS__);

extern "C"
JNIEXPORT void JNICALL
Java_com_shz_ffmpeg_MyVideoView_render(
        JNIEnv *env,
        jobject instance, jstring path, jobject surface) {

    const char *input = env->GetStringUTFChars(path, 0);
    //ffmpeg context
    av_register_all();
    avformat_network_init();
    AVFormatContext *pAVFormatContext =avformat_alloc_context();//replace of avFormatContext
    //根据视频路径 打开上下文
    avformat_open_input(&pAVFormatContext,input,0,0);
    //寻找流信息
    avformat_find_stream_info(pAVFormatContext,0);
    int video_stream_idx = -1;

    for (int i = 0; i < pAVFormatContext->nb_streams; ++i) {
        //找到视频流packet
        if(pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            break;
        }
    }
    AVCodecContext *pAVCodecContext = pAVFormatContext->streams[video_stream_idx]->codec;
    //通过解码器上下文 找到解码器
    AVCodec *pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    if(avcodec_open2(pAVCodecContext, pAVCodec,NULL) < 0) {
        return;
    }
    //申请帧缓冲区
    int out_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pAVCodecContext->width,pAVCodecContext->height, 1);
    uint8_t *out_buffer = (uint8_t *) malloc(sizeof(uint8_t) * out_size);
    //申请解压缩后的frame ffmpeg
    AVFrame *pFrame = av_frame_alloc();
    // rgb frame
    AVFrame *pRGB_Frame = av_frame_alloc();
    //rgb frame 分配缓冲区并用读取的文件数据填充
    av_image_fill_arrays(pRGB_Frame->data,pRGB_Frame->linesize,out_buffer, AV_PIX_FMT_RGBA,
                        pAVCodecContext->width, pAVCodecContext->height,1);
    //
    SwrContext *pSwrContext = (SwrContext *) sws_getContext(pAVCodecContext->width, pAVCodecContext->height,
                    pAVCodecContext->pix_fmt, pAVCodecContext->width, pAVCodecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);

    ANativeWindow *pANativeWindow = (ANativeWindow *) ANativeWindow_fromSurface(env, surface);

    ANativeWindow_Buffer window_buffer;
    AVPacket *pAVPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    int frameCount = 0, got_frame;
    //文件视频,读完的时候<0
    while (av_read_frame(pAVFormatContext,pAVPacket) >= 0){//1.读取压缩的Packet帧数据
        //packet 只要视频的packet
        LOGI("解码 %d帧",frameCount++);
        if(pAVPacket->stream_index == video_stream_idx){
            //解压缩 packet
            avcodec_decode_video2(pAVCodecContext, pFrame, &got_frame, pAVPacket);//2.解压缩packet为帧数据
            if(got_frame){
                LOGI("got_frame %d video_stream_idx %d",got_frame,video_stream_idx);
                sws_scale((SwsContext *) pSwrContext, (const uint8_t *const *)pFrame->data, pFrame->linesize,
                          0, pAVCodecContext->height, pRGB_Frame->data, pRGB_Frame->linesize);
                // rgb_frame
                ANativeWindow_setBuffersGeometry(pANativeWindow,pAVCodecContext->width,pAVCodecContext->height,
                                                 WINDOW_FORMAT_RGBA_8888);
                ANativeWindow_lock(pANativeWindow, &window_buffer,NULL);
                uint8_t *firstWindow = (uint8_t *)window_buffer.bits;//surface 窗口首地址
                uint8_t *firstRGBFrame = pRGB_Frame->data[0]; //RGB的首地址
                int windowLineByte = window_buffer.stride * 4; //每个像素含RGBA 4个通道
                int rgbLineByte = pRGB_Frame->linesize[0];
                for (int i = 0; i < pAVCodecContext->height; ++i) {
                    memcpy(firstWindow + i*windowLineByte,firstRGBFrame+ i*rgbLineByte,rgbLineByte);//3.绘制到显示窗口surface
                }
                //一幅画面绘制完成
                ANativeWindow_unlockAndPost(pANativeWindow);
                usleep(1000 * 16);//约60帧每秒
            }

        }
        av_free_packet(pAVPacket);
    }
    ANativeWindow_release(pANativeWindow);
    av_frame_free(&pFrame);
    av_frame_free(&pRGB_Frame);
    avcodec_close(pAVCodecContext);
    avformat_free_context(pAVFormatContext);

    env->ReleaseStringUTFChars(path,input);//释放字符串
}
