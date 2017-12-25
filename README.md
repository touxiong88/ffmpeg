# ffmpeg
我的CSDN http://blog.csdn.net/touxiong/article/details/78875657

Androidstudio+ffmpeg+camke播放器 编译的ffmpeg成so库使用cmake加载so及cpp,具有代码自动补全,一键编译java cpp功能的 ffmpeg Android平台播放器源码
视频直播核心技术-视频解码与NDK原声绘制
![这里写图片描述](http://img.blog.csdn.net/20171225150757523?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvdG91eGlvbmc=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

![这里写图片描述](http://img.blog.csdn.net/20171225150836212?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvdG91eGlvbmc=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
FFmpeg 视频解码 源码
C/C++开源视频处理库

C++ 异常的处理
安全类型的转换

 libavcodec/ 编解码
 libavdevice/ 设备
 libavfilter/ 滤镜
 libavformat/ 文件格式
 libavresample/ 重采样
 libavutil/ 工具类
 libpostproc/
 libswscale/ 缩放

![这里写图片描述](http://img.blog.csdn.net/20171225150908986?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvdG91eGlvbmc=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

![这里写图片描述](http://img.blog.csdn.net/20171225150924897?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvdG91eGlvbmc=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
so库
.h 头文件

cmake 链接库
${log-lib}
${android-lib}


[demo下载地址](http://download.csdn.net/download/touxiong/10169810)

cpp解码源码

```
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

```

arm平台加载libavcodec-56 有点小问题,播放不了,在arm64平台OPPO R9S上已经验证完美播放.



[ffmpeg源码编译android版本](http://blog.csdn.net/touxiong/article/details/78596520)

![这里写图片描述](http://img.blog.csdn.net/20171225150949068?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvdG91eGlvbmc=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
