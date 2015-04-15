#include "OpenCVStream.hpp"

OpenCVStream::OpenCVStream()
{

}

OpenCVStream::~OpenCVStream()
{

}

void
OpenCVStream::Start(cv_callback cv_cb, av_callback av_cb)
{
    cv::Mat capFrame;
    cv::Mat rgbFrame;
    cv::Mat flippedFrame;

    this->cap = new cv::VideoCapture(0);

    this->cap->set(CV_CAP_PROP_FOURCC, CV_FOURCC('H', '2', '6', '4'));
//    this->cap->set(CV_CAP_PROP_CONVERT_RGB , false);
//    this->cap->set(CV_CAP_PROP_FPS , 60);

    if (!cap->isOpened()) {
        // check if video device has been initialised`
        std::cout << "cannot open camera" << std::endl;
    }

    AVFrame *avFrame = av_frame_alloc();
    int numBytes = avpicture_get_size(PIX_FMT_RGB24, 1280, 720);
    uint8_t * frame2_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture*)avFrame, frame2_buffer, PIX_FMT_RGB24, 1280, 720);

    try{
        this->isRunning = true;
        while (this->isRunning) {
            boost::this_thread::interruption_point();
            this->cap->read(capFrame);

            if(!capFrame.empty()){
                // Flipped for mirror effect
                //::flip(capFrame, flippedFrame, 1);
                flippedFrame = capFrame;
                if(cv_cb)
                    cv_cb(flippedFrame);

                if(av_cb){
                    // Convert param cvFrame from BGR to RGB
                    cv::cvtColor(flippedFrame, rgbFrame, CV_BGR2RGB);

                    // This line did not work correctly, colors were jacked.
                    // Convert colorFrame from RGB to YCrCb (YUV) into original param
                    //cv::cvtColor(colorFrame, yuvFrame, CV_RGB2YCrCb);

                    // Resized to half the natural size
                    //v::resize(frame, resizedFrame, Size(), 1, 1, INTER_CUBIC);

                    // Grab cv::Mat to cv::Size is easier to read than cvFrame.rows or cvFrame.cols
                    cv::Size cvFrameSize = rgbFrame.size();

                    int w = cvFrameSize.width;
                    int h = cvFrameSize.height;

                    avpicture_fill((AVPicture*)avFrame,
                                  (uint8_t *)rgbFrame.data,
                                  AV_PIX_FMT_RGB24,
                                  w, h);

                    avFrame->format = AV_PIX_FMT_RGB24;
                    avFrame->height = h;
                    avFrame->width = w;

                    av_cb(avFrame);
                }

                if (cv::waitKey(25) >= 0)
                    break;
            }
        }
    }
    catch(boost::thread_interrupted){
        std::cout << "closing camera..." << std::endl;
    }
    catch(std::exception ex){

        std::cout << "unexpected err " << ex.what() << std::endl;
    }
}

void
OpenCVStream::Stop(){
    this->isRunning = false;
    cap->release();
    delete this->cap;
}
