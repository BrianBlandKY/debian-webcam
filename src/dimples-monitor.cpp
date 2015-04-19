#include <iostream>
#include <string>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

/* Temporary Dependencies */
#include <opencv2/opencv.hpp>
#include <time.h>

#include "Video4Linux2.hpp"
#include "OpenCVPreview.hpp"

#include <math.h>
extern "C"{

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>

}

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

using namespace std;

FILE *f;
AVCodec *codec;
AVPacket pkt;
AVCodecContext *c= NULL;
AVFrame *frame;
uint8_t endcode[] = { 0, 0, 1, 0xb7 };
int got_output, ret, i, bufferSize;

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
	FILE *pFile;
    char szFilename[32];
	int  y;

     // Open file
    sprintf(szFilename, "frame.ppm", iFrame);
    pFile=fopen(szFilename, "wb");
   if(pFile==NULL)
       return;
    // Write header
   fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  // Write pixel data
  for(y=0; y<height; y++)
      fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

 // Close file
  fclose(pFile);
}

void callback(io::Message msg){
	av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    frame->data[0] = (uint8_t*)msg.buffer;

    //avcodec_encode_video(c, msg.buffer, bufferSize,frame);

	/* encode the image */
	ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
	if (ret < 0) {
		fprintf(stderr, "Error encoding frame\n");
		exit(1);
	}

	if (got_output) {
		printf("Write frame %3d (size=%5d)\n", i, pkt.size);
		fwrite(pkt.data, 1, pkt.size, f);
		av_free_packet(&pkt);
	}

    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        fflush(stdout);

        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }
}

void
ThreadStart(Video4Linux2* device)
{
	try
	{
		device->Start(callback, nullptr);
	}
	catch(boost::thread_interrupted &err){
		std::cout << "closing camera..." << std::endl;
	}
	catch(std::exception &ex){
		std::cout << "unexpected err " << ex.what() << std::endl;
	}
}

int main()
{
	try
	{
		Video4Linux2* device = new Video4Linux2("/dev/video0");

		device->Init();

		avcodec_register_all();

		char* filename = "test.mpg";
		int codec_id = CODEC_ID_MPEG2VIDEO; //CODEC_ID_RAWVIDEO;//AV_CODEC_ID_H264; //CODEC_ID_MPEG2VIDEO; //CODEC_ID_MJPEG;

		printf("Encode video file %s\n", filename);

		/* find the mpeg1 video encoder */
		codec = avcodec_find_encoder(codec_id);
		if (!codec) {
			fprintf(stderr, "Codec not found\n");
			exit(1);
		}

		c = avcodec_alloc_context3(codec);
		if (!c) {
			fprintf(stderr, "Could not allocate video codec context\n");
			exit(1);
		}

		/* put sample parameters */
		c->bit_rate = 400000;
		/* resolution must be a multiple of two */
		c->width = device->GetSize().width;
		c->height = device->GetSize().height;
		/* frames per second */
		c->time_base = (AVRational){1,25};
		/* emit one intra frame every ten frames
		 * check frame pict_type before passing frame
		 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
		 * then gop_size is ignored and the output of encoder
		 * will always be I frame irrespective to gop_size
		 */
		c->gop_size = 10;
		c->max_b_frames = 1;
		c->pix_fmt = AV_PIX_FMT_YUV420P;// PIX_FMT_NONE; //AV_PIX_FMT_YUV420P;// AV_PIX_FMT_YUV420P;

		if (codec_id == AV_CODEC_ID_H264)
			av_opt_set(c->priv_data, "preset", "slow", 0);

		/* open it */
		if (avcodec_open2(c, codec, NULL) < 0) {
			fprintf(stderr, "Could not open codec\n");
			exit(1);
		}

		f = fopen(filename, "wb");
		if (!f) {
			fprintf(stderr, "Could not open %s\n", filename);
			exit(1);
		}

		frame = av_frame_alloc();
		if (!frame) {
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = c->pix_fmt;
		frame->width  = c->width;
		frame->height = c->height;

		/* the image can be allocated by any means and av_image_alloc() is
		 * just the most convenient way if av_malloc() is to be used */
		bufferSize = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);

		if (ret < 0) {
			fprintf(stderr, "Could not allocate raw picture buffer\n");
			exit(1);
		}

		//device->Start(&callback, nullptr);

		boost::thread(&ThreadStart, device);

		string input;
		while(input != "x")
		{
			cin >> input;
			cout << endl;
		}

		device->Stop();

		 /* add sequence end code to have a real mpeg file */
		fwrite(endcode, 1, sizeof(endcode), f);
		fclose(f);

		avcodec_close(c);
		av_free(c);
		av_freep(&frame->data[0]);
		av_frame_free(&frame);

		return 0;
	}
	catch(std::exception &ex){
		cout << ex.what() << endl;
		return 1;
	}
}


//	OpenCVPreview* preview = new OpenCVPreview(device);
//
//	preview->Start();
//

//
//	preview->Stop();
