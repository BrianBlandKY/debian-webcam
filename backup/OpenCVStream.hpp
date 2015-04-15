#ifndef OPENCVSTREAM_HPP
#define OPENCVSTREAM_HPP

#include "Stream.hpp"

#include <opencv2/opencv.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <exception>

typedef void cv_callback(cv::Mat frame);
typedef void av_callback(AVFrame* frame);

class OpenCVStream : public Stream
{
public:
    OpenCVStream();
    ~OpenCVStream();
    void Start(cv_callback cv_cb, av_callback av_cb);
    void Stop();
private:
    cv::VideoCapture *cap;
    bool isRunning = false;
    cv::Mat getFrame();
};

#endif // OPENCVSTREAM_HPP
