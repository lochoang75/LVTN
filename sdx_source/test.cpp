#include <stdio.h>
#include <stdlib.h>

#if __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/videoio.hpp"

#include "hls_stream.h"
#include "ap_int.h"

#include "common/xf_common.h"
#include "common/xf_utility.h"
#include "imgproc/xf_colorthresholding.hpp"
#include "imgproc/xf_erosion.hpp"
#include "imgproc/xf_inrange.hpp"

#define __ARM_NEON__
#define __ARM_NEON
#else
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/videoio.hpp"
#endif
#include "detect_lane.h"
#include <chrono>
#include <time.h>

using namespace std::chrono;
using namespace std;

cv::Mat run_app(cv::Mat in_img ){
	detectLane(in_img);
	cv::Mat birdEyes = birdViewTransform(in_img);

	//auto start = high_resolution_clock::now();
	fillLane(birdEyes);
	//auto stop = high_resolution_clock::now();
	//auto duration = duration_cast<microseconds>(stop - start);
	//cout<<"FillLane elapse "<<duration.count()<<endl;

	cv::Mat tracking = cv::Mat::zeros(birdEyes.size(), CV_8UC3);
	//start = high_resolution_clock::now();
	slidingWindow(birdEyes, tracking);
	//stop = high_resolution_clock::now();
	//duration = duration_cast<microseconds>(stop - start);
	//cout<<"Sliding window elapse "<<duration.count()<<endl;
	return tracking;
}

void video() {
	time_t start, end;
	start = time(0);
	int tick = 0;
	int frame = 0;
	int temp = 0;
	cv::VideoWriter video1("outcpp1.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, cv::Size(240, 320));
	for (int idx = 11; idx <= 4200; idx++) {
		frame++;
		string name = to_string(idx);
		string savepath = "/home/root/image/t" + name + ".jpg";
		cv::Mat img;
		cout << idx << endl;
		img = cv::imread(savepath);
		cv::Mat res = run_app(img);
		end = time(0) - start;
		if (end >= 1) {
			cv::putText(res, cv::format("%d", frame), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
			start = time(0);
			temp = frame;
			frame = 0;
		}
		else {
			cv::putText(res, cv::format("%d", temp), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
		}
		video1.write(res);
	}
}

int main(int argc, char** argv)
{
	//auto start_app = high_resolution_clock::now();

	video();

	//auto end_app = high_resolution_clock::now();
	//auto duration = duration_cast<microseconds>(end_app - start_app);
	//cout<<"running elapse "<<duration.count()<<endl;

	return 0;
}
