#include "detect_lane.h"
#include "xf_inrange_config.h"
#include "xf_erosion_config.h"
void detectLane(cv::Mat &img) {
	static xf::Mat<XF_8UC1,H,W,NPIX> imgInput(img.rows,img.cols);

	cv::cvtColor(img, img, CV_BGR2GRAY);
	imgInput.copyTo(img.data);
	xf::Mat<XF_8UC1,H,W,NPIX> img_gray(img.rows,img.cols);
	xf::Mat<XF_8UC1,H,W,NPIX> img_gray1(img.rows,img.cols);
	short int lower_thresh=180;
	short int upper_thresh=254;
//	xf::RGB2HSV< XF_8UC4,H, W,NPIX >(imgInput, img_hsv);
//	xf::Mat<XF_8UC4, H, W, NPIX> img_thresholded(in_img.rows,in_img.cols);
	inrange_accel(imgInput, lower_thresh, upper_thresh, img_gray);

	cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS,
			cv::Size(2 * erosionSize + 1, 2 * erosionSize + 1),
			cv::Point(erosionSize, erosionSize));

	#if __SDSCC__
		unsigned char *structure_element=(unsigned char *)sds_alloc_non_cacheable(sizeof(unsigned char)*FILTER_SIZE*FILTER_SIZE);
	#else
		unsigned char structure_element[FILTER_SIZE*FILTER_SIZE];
	#endif

	for(int i=0;i<(FILTER_SIZE*FILTER_SIZE);i++)
	{
		structure_element[i]=element.data[i];
	}
	//erosion_accel(img_gray, img_gray1, structure_element);

	img.data = img_gray.copyFrom();
}
