#include "VideoRecorder.hpp"
#include <iostream>

VideoRecorder::VideoRecorder(io::IODevice* device){
	filename = "";
	this->muxTool = new MuxTool();
	this->device = device;
	this->frame = nullptr;
}

VideoRecorder::~VideoRecorder(){
	delete muxTool;
}

void
VideoRecorder::Callback(io::Message msg){
	try{
		VideoRecorder* recorder = (VideoRecorder*)msg.data;
		AVFrame* frame = recorder->GetFrame();

		avpicture_fill((AVPicture*)frame,
					  (uint8_t *)msg.buffer,
					  PIX_FMT_YUVJ422P, //PIX_FMT_RGB24, //PIX_FMT_YUVJ420P
					  msg.size.width,
					  msg.size.height);

		frame->format = PIX_FMT_YUVJ422P;
		frame->height = msg.size.height;
		frame->width = msg.size.width;

		recorder->muxTool->WriteVideoFrame(frame);
		//recorder->muxTool->SaveFrame("test.ppm", frame, msg.size.width, msg.size.height);
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
	io::Size size = device->GetSize();

	muxTool->Init(this->filename, size.width, size.height);

    frame = av_frame_alloc();
    int numBytes = avpicture_get_size(PIX_FMT_RGB24, size.width, size.height);
    uint8_t * frame2_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture*)frame, frame2_buffer, PIX_FMT_RGB24, size.width, size.height);

	cameraThread = boost::thread(&VideoRecorder::ThreadStart, this);
}

void
VideoRecorder::Stop()
{
	muxTool->CloseStream();
	device->Stop();
	cameraThread.interrupt();
}

AVFrame*
VideoRecorder::GetFrame()
{
	return frame;
}
