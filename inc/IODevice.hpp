#ifndef IODEVICE_HPP
#define IODEVICE_HPP

namespace io {
	struct Size
	{
		int height;
		int width;
	};

	struct Message{
		Size size;
		void* buffer;
		void* data;
	};

	typedef void io_callback(Message);

	class IODevice{
	public:
		virtual ~IODevice() {};
		virtual int Start(io_callback cb, void* data) = 0;
		virtual void Stop() = 0;
		virtual Size GetSize() = 0;
	};
}

#endif // IODEVICE_HPP
