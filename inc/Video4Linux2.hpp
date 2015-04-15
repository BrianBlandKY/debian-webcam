#ifndef VIDEO4LINUX2_HPP_
#define VIDEO4LINUX2_HPP_

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <exception>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "IODevice.hpp"

using namespace io;

class Video4Linux2 : virtual public IODevice
{
public:
	Video4Linux2();
    ~Video4Linux2();
    void Start(const char* device, io_callback cb, void* data);
    void Start(io_callback cb, void* data);
    void Stop();
    io::Size GetSize();

private:
    // Default camera settings for best FrameRate (25+ FPS)
    // 1280 x 720 will reduce FPS by 1/2 (not ideal)
    bool isRunning;
    Size size;
    uint8_t *buffer;
    void* data;
    static int xioctl(int fd, int request, void *arg);
    int PrintCameraInfo(int fd);
    int InitBufferMap(int fd);
    int CaptureImage(int fd, io_callback cb);
    int SetPixelFormat(int fd);
};

#endif /* VIDEO4LINUX2_HPP_ */
