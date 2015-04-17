#include "OpenCVPreview.hpp"

OpenCVPreview::OpenCVPreview(io::IODevice* device){
	this->device = device;
}

OpenCVPreview::~OpenCVPreview(){

}

void
OpenCVPreview::Callback(io::Message msg){
	IplImage* frame;
	CvMat cvmat = cvMat(msg.size.height, msg.size.width, CV_8UC3, msg.buffer);

	frame = cvDecodeImage(&cvmat, 1);
	cvNamedWindow("window",CV_WINDOW_AUTOSIZE);
	cvShowImage("window", frame);
	cvWaitKey(3);
	//cvSaveImage("image.jpg", frame, 0);
}

void
OpenCVPreview::ThreadStart()
{
	try
	{
		device->Start(&OpenCVPreview::Callback, (void*)this);
	}
	catch(boost::thread_interrupted &err){
		std::cout << "closing camera..." << std::endl;
	}
	catch(std::exception &ex){
		std::cout << "unexpected err " << ex.what() << std::endl;
	}
}

void
OpenCVPreview::Start()
{
	cameraThread = boost::thread(&OpenCVPreview::ThreadStart, this);
}

void
OpenCVPreview::Stop()
{
	device->Stop();
	cameraThread.interrupt();
}

