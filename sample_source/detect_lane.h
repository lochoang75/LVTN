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
void detectLane(cv::Mat &img);
