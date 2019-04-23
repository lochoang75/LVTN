
#include <stdio.h>
#include <stdlib.h>

#if __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"

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
#endif
#include "detect_lane.h"





int main(int argc, char** argv)
{
	cv::Mat in_img;
	/*  reading in the color image  */
	in_img = cv::imread(argv[1]);

	if (in_img.data == NULL)
	{
		fprintf(stderr,"Cannot open image at %s\n",argv[1]);
		return 0;
	}
	cv::imshow("Source", in_img);
	detectLane(in_img);
	cv::imshow("Result", in_img);
	cv::waitKey(0);
	return 0;
}
