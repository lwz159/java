#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <stdint.h>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
}

// Android 打印 Log
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, "player", FORMAT, ##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG, "player", FORMAT, ##__VA_ARGS__);

extern "C" {
JNIEXPORT void JNICALL Java_com_example_ffmpegstudy_MainActivity_playVideo(
        JNIEnv *env,
        jobject thiz, jstring path, jobject surface) {
    int result;
    const char *pcPath = env->GetStringUTFChars(path, NULL);
    /* 注册FFmpeg组件 */
    av_register_all();
    /* 初始化AVFormatContext上下文 */
    AVFormatContext *pFormatContext = avformat_alloc_context();
    /* 打开视频文件 */
    result = avformat_open_input(&pFormatContext, pcPath, NULL, NULL);
    if (result < 0) {
        LOGE("Player Error : Can not open video file");
        return;
    }
    /* 查找视频文件的流信息 */
    result = avformat_find_stream_info(pFormatContext, NULL);
    if (result < 0) {
        LOGE("Player Error : Can not find video file stream info");
        return;
    }
    /* 查找视频编码器 */
    int iVideoStreamIndex = -1;
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        /* 匹配视频流 */
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            iVideoStreamIndex = i;
        }
    }
    /* 没找到视频流 */
    if (iVideoStreamIndex == -1) {
        LOGE("Player Error : Can not find video stream");
        return;
    }

    /* 初始化视频编码器上下文 */
    AVCodecContext *pVideoCodecContext = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pVideoCodecContext,
                                  pFormatContext->streams[iVideoStreamIndex]->codecpar);
    /* 初始化视频解码器 */
    AVCodec *pVideoCodec = avcodec_find_decoder(pVideoCodecContext->codec_id);
    if (pVideoCodec == NULL) {
        LOGE("Player Error : Can not find video codec");
        return;
    }
    /* 发开视频解码器 */
    result = avcodec_open2(pVideoCodecContext, pVideoCodec, NULL);
    if (result < 0) {
        LOGE("Player Error : Can not find video stream");
        return;
    }
    /* 获取视频的宽高 */
    int videoWidth = pVideoCodecContext->width;
    int videoHeight = pVideoCodecContext->height;
    /* 初始化NativeWindow用于播放视频 */
    ANativeWindow *pNativeWindow = ANativeWindow_fromSurface(env, surface);
    if (pNativeWindow == NULL) {
        LOGE("Player Error : Can not create native window");
        return;
    }
    // 通过设置宽高限制缓冲区中的像素数量，而非屏幕的物理显示尺寸。
    // 如果缓冲区与物理屏幕的显示尺寸不相符，则实际显示可能会是拉伸，或者被压缩的图像
    result = ANativeWindow_setBuffersGeometry(pNativeWindow, videoWidth, videoHeight,
                                              WINDOW_FORMAT_RGBA_8888);
    if (result < 0) {
        LOGE("Player Error : Can not set native window buffer");
        ANativeWindow_release(pNativeWindow);
        return;
    }
    // 定义绘图缓冲区
    ANativeWindow_Buffer windowBuffer;
    // 声明数据容器 有3个
    // R5 解码前数据容器 Packet 编码数据
    AVPacket *packet = av_packet_alloc();
    // R6 解码后数据容器 Frame 像素数据 不能直接播放像素数据 还要转换
    AVFrame *pFrame = av_frame_alloc();
    // R7 转换后数据容器 这里面的数据可以用于播放
    AVFrame *pRgbaGrame = av_frame_alloc();
    // 数据格式转换准备
    // 输出 Buffer
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, videoWidth, videoHeight, 1);
    // R8 申请 Buffer 内存
    uint8_t * out_buffer = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));
    av_image_fill_arrays(pRgbaGrame->data, pRgbaGrame->linesize, out_buffer, AV_PIX_FMT_RGBA,
                         videoWidth, videoHeight, 1);
    // R9 数据格式转换上下文
    struct SwsContext *data_convert_context = sws_getContext(
            videoWidth, videoHeight, pVideoCodecContext->pix_fmt,
            videoWidth, videoHeight, AV_PIX_FMT_RGBA,
            SWS_BICUBIC, NULL, NULL, NULL);

    // 开始读取帧
    while (av_read_frame(pFormatContext, packet) >= 0) {
        // 匹配视频流
        if (packet->stream_index == iVideoStreamIndex) {
            // 解码
            result = avcodec_send_packet(pVideoCodecContext, packet);
            if (result < 0 && result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
                LOGE("Player Error : codec step 1 fail");
                return;
            }
            result = avcodec_receive_frame(pVideoCodecContext, pFrame);
            if (result < 0 && result != AVERROR_EOF) {
                LOGE("Player Error : codec step 2 fail");
                return;
            }
            // 数据格式转换
            result = sws_scale(
                    data_convert_context,
                    (const uint8_t *const *) pFrame->data, pFrame->linesize,
                    0, videoHeight,
                    pRgbaGrame->data, pRgbaGrame->linesize);
            if (result <= 0) {
                LOGE("Player Error : data convert fail %d", result);
                return;
            }
            LOGD("play normal");
            // 播放
            result = ANativeWindow_lock(pNativeWindow, &windowBuffer, NULL);
            if (result < 0) {
                LOGE("Player Error : Can not lock native window");
            } else {
                // 将图像绘制到界面上
                // 注意 : 这里 rgba_frame 一行的像素和 window_buffer 一行的像素长度可能不一致
                // 需要转换好 否则可能花屏
                uint8_t * bits = (uint8_t *) windowBuffer.bits;
                for (int h = 0; h < videoHeight; h++) {
                    memcpy(bits + h * windowBuffer.stride * 4,
                           out_buffer + h * pRgbaGrame->linesize[0],
                           pRgbaGrame->linesize[0]);
                }
                ANativeWindow_unlockAndPost(pNativeWindow);
            }
        }
        // 释放 packet 引用
        av_packet_unref(packet);
    }

    // 释放 R9
    sws_freeContext(data_convert_context);
    // 释放 R8
    av_free(out_buffer);
    // 释放 R7
    av_frame_free(&pRgbaGrame);
    // 释放 R6
    av_frame_free(&pFrame);
    // 释放 R5
    av_packet_free(&packet);
    // 释放 R4
    ANativeWindow_release(pNativeWindow);
    // 关闭 R3
    avcodec_close(pVideoCodecContext);
    // 释放 R2
    avformat_close_input(&pFormatContext);

    env->ReleaseStringUTFChars(path, pcPath);
}

JNIEXPORT void JNICALL Java_com_example_ffmpegstudy_MainActivity_demuxToGetAudio(
        JNIEnv *env, jobject thiz, jstring path, jstring targetPath) {
    const char *pcSrcPath = env->GetStringUTFChars(path, NULL);
    const char *pcTargetPath = env->GetStringUTFChars(targetPath, NULL);
    /* 注册FFmpeg组件 */
    av_register_all();
    AVFormatContext *pInputFormatContext = NULL;
    int iRet = avformat_open_input(&pInputFormatContext, pcSrcPath, NULL, NULL);
    if (iRet < 0) {
        LOGE("can not open file %s\n", pcSrcPath);
        return;
    }
    /* 查找视频编码器 */
    int iVideoStreamIndex = -1;
    for (int i = 0; i < pInputFormatContext->nb_streams; i++) {
        /* 匹配音频流 */
        if (pInputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            iVideoStreamIndex = i;
        }
    }
    /* 没找到音频流 */
    if (iVideoStreamIndex == -1) {
        LOGE("Player Error : Can not find audio stream");
        return;
    }

    AVStream *pInputStream = pInputFormatContext->streams[iVideoStreamIndex];
    AVCodecParameters *pInputCodecPar = pInputStream->codecpar;

    AVFormatContext *pOutFormatContext = NULL;
    iRet = avformat_alloc_output_context2(&pOutFormatContext, NULL, NULL, pcTargetPath);
    if (pOutFormatContext == NULL) {
        LOGE("play error: could not create output context %d for %s", iRet, pcTargetPath);
        iRet = AVERROR_UNKNOWN;
        return;
    }
    AVStream *pOutStream = avformat_new_stream(pOutFormatContext, NULL);
    if (pOutStream == NULL) {
        LOGE("创建输出流失败");
        return;
    }
    if ((iRet = avcodec_parameters_copy(pOutStream->codecpar, pInputCodecPar)) < 0) {
        LOGE("拷贝编码参数失败");
        return;
    }
    if ((iRet = avio_open(&pOutFormatContext->pb, pcTargetPath, AVIO_FLAG_WRITE)) < 0) {
        LOGE("can not open file %s", pcTargetPath);
        return;
    }

    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    if (avformat_write_header(pOutFormatContext, NULL) < 0) {
        LOGE("can not write head\n");
        return;
    }
    LOGE("++++++++");
    while(av_read_frame(pInputFormatContext, &packet) == 0) {
        LOGE("%lld", packet.pts);
        if (packet.stream_index == iVideoStreamIndex) {
            //时间基计算，音频pts和dts一致
            packet.pts = av_rescale_q_rnd(packet.pts, pInputStream->time_base, pOutStream->time_base, (AV_ROUND_PASS_MINMAX));
            packet.dts = packet.pts;
            packet.duration = av_rescale_q(packet.duration, pInputStream->time_base, pOutStream->time_base);
            packet.pos = -1;
            packet.stream_index = 0;
            //将包写到输出媒体文件
            av_interleaved_write_frame(pOutFormatContext, &packet);
            //减少引用计数，避免内存泄漏
            av_packet_unref(&packet);
        }
    }

    //写尾部信息
    av_write_trailer(pOutFormatContext);

    //最后别忘了释放内存
    avformat_close_input(&pInputFormatContext);
    avio_close(pOutFormatContext->pb);
}


JNIEXPORT void JNICALL Java_com_example_ffmpegstudy_MainActivity_transferMp4ToAVI(
        JNIEnv *env,
        jobject thiz, jstring path, jstring targetPath) {
        AVOutputFormat *pOutputFormat = NULL;
        AVBitStreamFilterContext *pAvBitStreamFilter = NULL;
        /* 定义输入输出AVFormatContext */
        AVFormatContext *pInFormatContext = NULL, *pOutFormatContext = NULL;
        const char *pcInputFilePath = env->GetStringUTFChars(path, NULL);
        const char *pcOutputFilePath = env->GetStringUTFChars(targetPath, NULL);

        av_register_all();
        int iRet = 0;
        int frame_index = 0;
        // 打开输入文件
        if ((iRet = avformat_open_input(&pInFormatContext, pcInputFilePath, 0, 0)) < 0) {
            LOGE("play error: can not open inout file");
            goto end;
        }
        // 获取视频信息
        if ((iRet = avformat_find_stream_info(pInFormatContext, 0)) < 0) {
            LOGE("Failed to retrieve input stream information");
            goto end;
        }
        pAvBitStreamFilter = av_bitstream_filter_init("h264_mp4toannexb");
        av_dump_format(pInFormatContext, 0 , pcInputFilePath, 0);
        // 初始化输出视频码流的AVFormatContext
        iRet = avformat_alloc_output_context2(&pOutFormatContext, NULL, NULL, pcOutputFilePath);
        if (pOutFormatContext == NULL) {
            LOGE("play error: could not create output context %d for %s", iRet, pcOutputFilePath);
            iRet = AVERROR_UNKNOWN;
            goto end;
        }
        pOutputFormat = pOutFormatContext->oformat;
        for (int i = 0; i < pInFormatContext->nb_streams; i++) {
            /* 通过输入的AVStream创建输出的AVStream */
            AVStream *pInStream = pInFormatContext->streams[i];
            AVStream *pOutStream = avformat_new_stream(pOutFormatContext, pInStream->codec->codec);
            if (pOutStream == NULL) {
                LOGE("Failed allocating output stream");
                iRet = AVERROR_UNKNOWN;
                goto end;
            }
            if (avcodec_copy_context(pOutStream->codec, pInStream->codec) < 0) {
                LOGE("Failed to copy context from input to output stream codec context");
                goto end;
            }
            pOutStream->codec->codec_tag = 0;
            if (pOutFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
                pOutStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
        }
        // 输出信息
        av_dump_format(pOutFormatContext, 0, pcOutputFilePath, 1);
        /* 打开输出文件 */
        if (!(pOutFormatContext->flags & AVFMT_NOFILE)) {
            iRet = avio_open(&pOutFormatContext->pb, pcOutputFilePath, AVIO_FLAG_WRITE);
            /* 打开输出文件 */
            if (iRet < 0) {
                LOGE("could open output file %s", pcOutputFilePath);
                goto end;
            }
        }
        if (avformat_write_header(pOutFormatContext, NULL) < 0) {
            LOGE("Error occurred when opening output file");
            goto end;
        }


        while (1) {
            AVStream *pInputStream, *pOutStream;
            // get an av packet
            AVPacket packet;
            iRet = av_read_frame(pInFormatContext, &packet);
            if (iRet < 0) {
                break;
            }
            pInputStream = pInFormatContext->streams[packet.stream_index];
            pOutStream = pOutFormatContext->streams[packet.stream_index];

            // translate PTS/DTS
            packet.pts = av_rescale_q_rnd(packet.pts, pInputStream->time_base, pOutStream->time_base,
                                          (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            packet.dts = av_rescale_q_rnd(packet.dts, pInputStream->time_base, pOutStream->time_base,
                                          (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            packet.pos = -1;
            if (packet.stream_index == 0) {
                AVPacket fPacket = packet;
                int a = av_bitstream_filter_filter(pAvBitStreamFilter, pOutStream->codec, NULL, &fPacket.data,
                        &fPacket.size, packet.data, packet.size, packet.flags & AV_PKT_FLAG_KEY);
                packet.data = fPacket.data;
                packet.size = fPacket.size;
            }

            if (av_write_frame(pOutFormatContext, &packet) < 0) {
                LOGE("Error muxing packet");
                break;
            }
            LOGD("Write %8d frames to output file", frame_index);
            av_packet_unref(&packet);
            frame_index++;
        }
        av_write_trailer(pOutFormatContext);

    end:
        avformat_close_input(&pInFormatContext);
        /* 关闭输出 */
        if (pOutFormatContext && !(pOutFormatContext->flags & AVFMT_NOFILE)) {
            avio_close(pOutFormatContext->pb);
        }
        avformat_free_context(pOutFormatContext);
        return;
}

JNIEXPORT void JNICALL Java_com_example_ffmpegstudy_MainActivity_demuxToGetVideo(JNIEnv *pEnv,
                                                                                 jobject thiz, jstring path, jstring targetPath) {
    const char *pcPath = pEnv->GetStringUTFChars(path, NULL);
    const char *pcTargetPath = pEnv->GetStringUTFChars(targetPath, NULL);

    av_register_all();      // register
    AVFormatContext *pFormatContext = NULL;

    int iVideoIndex = -1;
    FILE *pVideoFile = fopen(pcTargetPath, "wb+");
    AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");

    if (avformat_open_input(&pFormatContext, pcPath, 0, 0) < 0) {
        LOGE("could not open input file %s\n", pcPath);
        goto end;
    }

    if (avformat_find_stream_info(pFormatContext, 0) < 0) {
        LOGE("could not retrieve input stream information");
        goto end;
    }

    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            iVideoIndex = i;
        }
    }

    AVPacket pkt;
    while(av_read_frame(pFormatContext, &pkt) >= 0) {
        if (pkt.stream_index == iVideoIndex) {
            av_bitstream_filter_filter(h264bsfc, pFormatContext->streams[iVideoIndex]->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
            fwrite(pkt.data, 1, pkt.size, pVideoFile);
        }
        av_free_packet(&pkt);
    }

    av_bitstream_filter_close(h264bsfc);

    end:

        fclose(pVideoFile);
        avformat_close_input(&pFormatContext);

        pEnv->ReleaseStringUTFChars(path, pcPath);
        pEnv->ReleaseStringUTFChars(targetPath, pcTargetPath);
}

}