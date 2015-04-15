#ifndef VIDEORECORDER_HPP_
#define VIDEORECORDER_HPP_

extern "C"{
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    //#include <libavutil/timestamp.h>
    #include <libavutil/mathematics.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>

#include "IODevice.hpp"

class VideoRecorder
{
public:
	VideoRecorder(io::IODevice* device);
    ~VideoRecorder();
    void Start(const char*);
    void Stop();

private:
    io::IODevice* device = nullptr;
    AVFrame* avFrame = nullptr;
    const char* filename;
    static void Callback(io::Message);
    void ThreadStart();
};

#endif /* VIDEORECORDER_HPP_ */
