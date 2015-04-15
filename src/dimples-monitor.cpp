#include <iostream>
#include <string>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

/* Temporary Dependencies */
#include <opencv2/opencv.hpp>
#include <time.h>

#include "../inc/Video4Linux2.hpp"
#include "../inc/OpenCVPreview.hpp"
#include "../inc/VideoRecorder.hpp"

using namespace std;

int main()
{
	Video4Linux2* device = new Video4Linux2();
	OpenCVPreview* preview = new OpenCVPreview(device);
	VideoRecorder* recorder = new VideoRecorder(device);

	preview->Start();
	recorder->Start("test.mp4");

	string input;
	while(input != "x")
	{
		cin >> input;
		cout << endl;
	}
	preview->Stop();
	recorder->Stop();

	//delete device;
	delete preview;

	return 0;
}
