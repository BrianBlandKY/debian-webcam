#ifndef VIDEORECORDER_HPP_
#define VIDEORECORDER_HPP_

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>

#include "IODevice.hpp"
#include "MuxTool.hpp"

class VideoRecorder
{
public:
	VideoRecorder(io::IODevice* device);
    ~VideoRecorder();
    void Start(const char*);
    void Stop();
    AVFrame* GetFrame();

private:
    io::IODevice* device;
    MuxTool* muxTool;
    AVFrame* frame;
    const char* filename;
    boost::thread cameraThread;
    static void Callback(io::Message);
    void ThreadStart();
};

#endif /* VIDEORECORDER_HPP_ */
