#include "../inc/OpenCVPreview.hpp"

OpenCVPreview::OpenCVPreview(io::IODevice* device){
	this->device = device;
}

OpenCVPreview::~OpenCVPreview(){

}

void
OpenCVPreview::Callback(io::Message msg){
	IplImage* frame;
	CvMat cvmat = cvMat(360, 640, CV_8UC3, msg.buffer);

	frame = cvDecodeImage(&cvmat, 1);
	cvNamedWindow("window",CV_WINDOW_AUTOSIZE);
	cvShowImage("window", frame);
	cvWaitKey(3);
	//cvSaveImage("image.jpg", frame, 0);
}

void
OpenCVPreview::ThreadStart()
{
	device->Start(&OpenCVPreview::Callback, (void*)this);
}

void
OpenCVPreview::Start()
{
	boost::thread cameraThread(&OpenCVPreview::ThreadStart, this);
}

void
OpenCVPreview::Stop()
{
	device->Stop();
}

