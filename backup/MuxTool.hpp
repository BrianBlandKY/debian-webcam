#ifndef MUXTOOL_HPP
#define MUXTOOL_HPP

#ifdef WIN64
#define snprintf _snprintf
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern "C"{
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/mathematics.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#define STREAM_FRAME_RATE 30 /* 25 images/s */
#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC

// a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;
    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;
    AVFrame *frame;
    float t, tincr, tincr2;
    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

class MuxTool
{
public:
    MuxTool();
    ~MuxTool();
    void Init(const char *filename, int width, int height);
    void CloseStream();
    int WriteVideoFrame(AVFrame *frame);
    void SaveFrame(const char* filename, AVFrame *pFrame, int width, int height);

private:
    static const bool is_ffmpeg_init = false;
    int width = 0;
    int height = 0;
    int frameIndex = 0;
    AVFormatContext *oc;
    AVOutputFormat *fmt;
    OutputStream video_st = OutputStream{0};
    AVCodec *video_codec;
    AVDictionary *opt;

    int WriteFrame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt);
    AVFrame* AllocPicture(enum AVPixelFormat pix_fmt, int width, int height);
    void AddStream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
    void OpenVideo(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg);

    // Temp
    AVFrame* GetVideoFrame(OutputStream *ost, AVFrame *frame);
};

#endif // MUXTOOL_HPP
