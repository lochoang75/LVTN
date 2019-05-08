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
#include "hls_stream.h"
#include "ap_int.h"
#include "common/xf_common.h"
#include "common/xf_utility.h"
#include "imgproc/xf_erosion.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#endif

#define W 480
#define H 320

#define NPIX XF_NPPC1


#define TYPE XF_8UC1
#define NPIX XF_NPPC1
#define erosionSize 1
#define KERNEL_SHAPE XF_SHAPE_CROSS
#define ITERATIONS 1
#define BIRDVIEW_WIDTH 240
#define BIRDVIEW_HEIGHT 320
#define skyLine 85
#define delta 20
#define point_max 16

void detectLane(cv::Mat &img);
cv::Mat birdViewTransform(const cv::Mat &src);
void fillLane(cv::Mat &src);
void slidingWindow(cv::Mat src, cv::Mat &tracking);
int nearestX(cv::Point* list, int position, double mean);
