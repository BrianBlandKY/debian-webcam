#include "../inc/Video4Linux2.hpp"

Video4Linux2::Video4Linux2():IODevice()
{
	isRunning = false;
	size.height = 360;
	size.width = 640;
    buffer = nullptr;
}

Video4Linux2::~Video4Linux2()
{
	delete buffer;
}

void Video4Linux2::Start(io_callback cb, void* data)
{
	Start("/dev/video0", cb, data);
}

void
Video4Linux2::Start(const char* device, io_callback cb, void* data)
{
	this->data = data;
	int fd;

	fd = open(device, O_RDWR);
	if (fd == -1)
	{
		perror("Opening video device");
		return; // 1
	}
	if(SetPixelFormat(fd))
		return;
	// Not required, general details
	if(PrintCameraInfo(fd))
		return;
	if(InitBufferMap(fd))
		return;

	try{
		do{
			if(CaptureImage(fd, cb))
				return;
		}while(isRunning);
	}
	catch(boost::thread_interrupted &err){
		std::cout << "closing camera..." << std::endl;
	}
	catch(std::exception &ex){
		std::cout << "unexpected err " << ex.what() << std::endl;
	}

	close(fd);

	return;
}

void
Video4Linux2::Stop(){
    this->isRunning = false;
    delete buffer;
}

int
Video4Linux2::xioctl(int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

int
Video4Linux2::SetPixelFormat(int fd)
{
	char fourcc[5] = {0};
	struct v4l2_format fmt = {};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 360;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	/*
	 * $ v4l2-ctl --list-devices (list all web cam devices)
	 * $ v4l2-ctl --list-formats (list web cam formats) (iSight->MJPEG only!)
	 */

	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
	{
		perror("Setting Pixel Format");
		return 1;
	}

	strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
	printf( "Selected Camera Mode:\n"
			"  Width: %d\n"
			"  Height: %d\n"
			"  PixFmt: %s\n"
			"  Field: %d\n",
			fmt.fmt.pix.width,
			fmt.fmt.pix.height,
			fourcc,
			fmt.fmt.pix.field);

	return 0;
}

int
Video4Linux2::PrintCameraInfo(int fd)
{
	struct v4l2_capability caps = {};
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps))
	{
		perror("Querying Capabilities");
		return 1;
	}

	printf( "Driver Caps:\n"
			"  Driver: \"%s\"\n"
			"  Card: \"%s\"\n"
			"  Bus: \"%s\"\n"
			"  Version: %d.%d\n"
			"  Capabilities: %08x\n",
			caps.driver,
			caps.card,
			caps.bus_info,
			(caps.version>>16)&&0xff,
			(caps.version>>24)&&0xff,
			caps.capabilities);


	struct v4l2_cropcap cropcap = {};
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (fd, VIDIOC_CROPCAP, &cropcap))
	{
		perror("Querying Cropping Capabilities");
		return 1;
	}

	printf( "Camera Cropping:\n"
			"  Bounds: %dx%d+%d+%d\n"
			"  Default: %dx%d+%d+%d\n"
			"  Aspect: %d/%d\n",
			cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
			cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
			cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

	int support_grbg10 = 0;

	struct v4l2_fmtdesc fmtdesc = {0};
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	char fourcc[5] = {0};
	char c, e;
	printf("  FMT : CE Desc\n--------------------\n");
	while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc))
	{
		strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
		if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
			support_grbg10 = 1;
		c = fmtdesc.flags & 1? 'C' : ' ';
		e = fmtdesc.flags & 2? 'E' : ' ';
		printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
		fmtdesc.index++;
	}

	return 0;
}

int
Video4Linux2::InitBufferMap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }

    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        perror("Querying Buffer");
        return 1;
    }

    buffer = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    printf("Image Length: %d\n", buf.bytesused);

    return 0;
}

int
Video4Linux2::CaptureImage(int fd, io_callback cb)
{
	fd_set fds;
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Query Buffer");
        return 1;
    }

    if(!isRunning)
    {
		if(-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type))
		{
			perror("Start Capture");
			return 1;
		}

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		isRunning = true;
    }
	struct timeval tv = {0};
	tv.tv_sec = 2;
	int r = select(fd+1, &fds, NULL, NULL, &tv);
	if(-1 == r)
	{
		perror("Waiting for Frame");
		return 1;
	}
	if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
	{
		perror("Retrieving Frame");
		return 1;
	}
	if(cb != nullptr){
		Message msg;
		msg.buffer = (void*)buffer;
		msg.size = size;
		msg.data = this->data;

		cb(msg);
	}

    return 0;
}

io::Size
Video4Linux2::GetSize()
{
	 return size;
}

