#ifndef OPENCVPREVIEW_HPP_
#define OPENCVPREVIEW_HPP_

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <opencv2/opencv.hpp>
#include <time.h>

#include "IODevice.hpp"

class OpenCVPreview
{
public:
    OpenCVPreview(io::IODevice* device);
    ~OpenCVPreview();
    void Start();
    void Stop();
private:
    io::IODevice* device;
    boost::thread cameraThread;
    void ThreadStart();
    static void Callback(io::Message);
};

#endif /* OPENCVPREVIEW_HPP_ */
