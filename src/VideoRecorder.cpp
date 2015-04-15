#include "VideoRecorder.hpp"

VideoRecorder::VideoRecorder(io::IODevice* device){
	this->filename = "";
	this->avFrame = nullptr;
	this->device = device;
}

VideoRecorder::~VideoRecorder(){

}

void
VideoRecorder::Callback(io::Message msg){
	try{
		VideoRecorder* recorder = (VideoRecorder*)msg.data;
		// Convert Frame and Record
//		// Grab cv::Mat to cv::Size is easier to read than cvFrame.rows or cvFrame.cols
//		cv::Size cvFrameSize = rgbFrame.size();
//
//		int w = cvFrameSize.width;
//		int h = cvFrameSize.height;
//
//		avpicture_fill((AVPicture*)avFrame,
//					  (uint8_t *)rgbFrame.data,
//					  AV_PIX_FMT_RGB24,
//					  w, h);
//
//		avFrame->format = AV_PIX_FMT_RGB24;
//		avFrame->height = h;
//		avFrame->width = w;

	}
	catch(boost::thread_interrupted &err){
		std::cout << "closing camera..." << std::endl;
	}
	catch(std::exception &ex){
		std::cout << "unexpected err " << ex.what() << std::endl;
	}
}

void
VideoRecorder::ThreadStart()
{
	device->Start(&VideoRecorder::Callback, (void*) this);
}

void
VideoRecorder::Start(const char* filename)
{
	this->filename = filename;
	io::Size size = this->device->GetSize();
	//this->avFrame = av_frame_alloc();
	this->avFrame = avcodec_alloc_frame();

	int numBytes = avpicture_get_size(PIX_FMT_RGB24, size.width, size.height);
	uint8_t * frame2_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	avpicture_fill((AVPicture*)avFrame, frame2_buffer, PIX_FMT_RGB24, size.width, size.height);

	boost::thread cameraThread(&VideoRecorder::ThreadStart, this);
}

void
VideoRecorder::Stop()
{
	device->Stop();
}

