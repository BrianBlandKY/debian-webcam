#include "MuxTool.hpp"

#include <iostream>
using namespace std;

MuxTool::MuxTool(){

}

MuxTool::~MuxTool(){

}

void MuxTool::Init(const char *filename, int width, int height)
{
    int ret = 0;
    this->width = width;
    this->height = height;
    this->frameIndex = 0;

    if(!this->is_ffmpeg_init)
        // Registers all FFMPEG/LibAV Codecs (Required)
        av_register_all();


    avformat_alloc_output_context2(&this->oc, NULL, NULL, filename);

    if (!this->oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&this->oc, NULL, "mpeg", filename);
    }

    if (!this->oc){
        printf("Could not alloc output format. \n");
        return;
    }

    this->fmt = this->oc->oformat;

    if (this->fmt->video_codec != AV_CODEC_ID_NONE) {
        AddStream(&this->video_st, this->oc, &this->video_codec, fmt->video_codec);
    }

    OpenVideo(this->oc, this->video_codec, &this->video_st, this->opt);

    av_dump_format(this->oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(this->fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&this->oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0){
            fprintf(stderr, "Could not open '%s': %s\n", filename, ret); //av_err2str(ret));
            return;
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(this->oc, &this->opt);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file: %s\n", ret);// av_err2str(ret));
        return;
    }
}

void MuxTool::CloseStream()
{
    av_write_trailer(this->oc);

    OutputStream *ost = &this->video_st;

    avcodec_close(ost->st->codec);
    av_frame_free(&ost->frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
    avformat_free_context(this->oc);
}

int MuxTool::WriteVideoFrame(AVFrame *frame2)
{
    int ret;
    AVCodecContext *c;
    int got_packet = 0;
    OutputStream *ost = &this->video_st;

    c = ost->st->codec;
    AVFrame* frame = GetVideoFrame(ost, frame2);

    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* a hack to avoid data copy with some raw video muxers */
        AVPacket pkt;
        av_init_packet(&pkt);
        if (!frame)
            return 1;
        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.stream_index  = ost->st->index;
        pkt.data          = (uint8_t *)frame;
        pkt.size          = sizeof(AVPicture);
        pkt.pts = pkt.dts = frame->pts;
        av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
        ret = av_interleaved_write_frame(oc, &pkt);
    } else {
        AVPacket pkt = { 0 };
        av_init_packet(&pkt);
        /* encode the image */

        //Calculate PTS: (1 / FPS) * sample rate * frame number
        //sample rate 90KHz is for h.264 at 30 fps
        frame->pts = (1.0 / 30) * 90 * this->frameIndex;
        this->frameIndex++;
        //frame->pts = c->frame_number;
        ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);

        if (ret < 0) {
            fprintf(stderr, "Error encoding video frame: %s\n", ret);//av_err2str(ret));
            exit(1);
        }
        if (got_packet) {
            pkt.stream_index = this->video_st.st->index;

            ret = WriteFrame(oc, &c->time_base, ost->st, &pkt);
        } else {
            ret = 0;
        }
    }
    if (ret < 0) {
        fprintf(stderr, "Error while writing video frame: %s\n", ret);// av_err2str(ret));
        exit(1);
    }
    return (frame || got_packet) ? 0 : 1;
}

void MuxTool::SaveFrame(const char* filename, AVFrame *pFrame, int width, int height)
{
    FILE *pFile;
    char szFilename[32];
    int  y;

    // Open file
    pFile=fopen(filename, "wb");
    if(pFile==NULL){
        fprintf(stdout, "Never Created File!");
        return;
    }

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, pFrame->linesize[0], pFile); // width*3

    // Close file
    fclose(pFile);
}

// Private
void MuxTool::OpenVideo(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;
    AVCodecContext *c = ost->st->codec;
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        fprintf(stderr, "Could not open video codec: %s\n", ret);//av_err2str(ret));
        exit(1);
    }

    /* allocate and init a re-usable frame */
    ost->frame = AllocPicture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
}

void MuxTool::AddStream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id)
{
    AVCodecContext *c;
    int i;
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }
    ost->st = avformat_new_stream(oc, *codec);
    if (!ost->st) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    ost->st->id = oc->nb_streams-1;
    c = ost->st->codec;
    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt  = (*codec)->sample_fmts ?
            (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        if ((*codec)->channel_layouts) {
            c->channel_layout = (*codec)->channel_layouts[0];
            for (i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        ost->st->time_base = AVRational{ 1, c->sample_rate };
        break;
    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;
        c->bit_rate = 9000000;
        /* Resolution must be a multiple of two. */
        c->width    = 1280;
        c->height   = 720;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->st->time_base = AVRational{ 1, STREAM_FRAME_RATE };
        c->time_base       = ost->st->time_base;
        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_PIX_FMT;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
    break;
    default:
        break;
    }
    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

int MuxTool::WriteFrame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    /* Write the compressed frame to the media file. */
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

AVFrame* MuxTool::AllocPicture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;
    picture = av_frame_alloc();
    if (!picture)
        return NULL;
    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;
    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }
    return picture;
}

AVFrame* MuxTool::GetVideoFrame(OutputStream *ost, AVFrame *frame)
{
    AVCodecContext *c = ost->st->codec;
    if (frame->format!= AV_PIX_FMT_YUV420P) {

        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(frame->width, frame->height,
                                          AV_PIX_FMT_RGB24, // AV_PIX_FMT_YUV420P,
                                          c->width, c->height,
                                          c->pix_fmt,
                                          SCALE_FLAGS, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr,
                        "Could not initialize the conversion context\n");
                exit(1);
            }
        }
        sws_scale(ost->sws_ctx,
                  (const uint8_t * const *)frame->data,
                  frame->linesize,
                  0, c->height,
                  ost->frame->data,
                  ost->frame->linesize);
    } else {
        ost->frame = frame;
    }
    ost->frame->pts = ost->next_pts++;
    return ost->frame;
}
