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

cv::Mat birdViewTransform(const cv::Mat &src)
{
	cv::Point2f src_vertices[4];

	int width = src.size().width;
	int height = src.size().height;

	src_vertices[0] = cv::Point(0, skyLine);
	src_vertices[1] = cv::Point(width, skyLine);
	src_vertices[2] = cv::Point(width, height);
	src_vertices[3] = cv::Point(0, height);

	cv::Point2f dst_vertices[4];
	dst_vertices[0] = cv::Point(0, 0);
	dst_vertices[1] = cv::Point(BIRDVIEW_WIDTH, 0);
	dst_vertices[2] = cv::Point(BIRDVIEW_WIDTH - 105, BIRDVIEW_HEIGHT);
	dst_vertices[3] = cv::Point(105, BIRDVIEW_HEIGHT);

	cv::Mat M = cv::getPerspectiveTransform(src_vertices, dst_vertices);

	cv::Mat dst(BIRDVIEW_HEIGHT, BIRDVIEW_WIDTH, CV_8UC3);
	cv::warpPerspective(src, dst, M, dst.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);

	return dst;
}

void fillLane(cv::Mat &src)
{
	cv::Scalar color[3];
	color[0] = cv::Scalar(255, 0, 0);
	color[1] = cv::Scalar(0, 255, 0);
	color[2] = cv::Scalar(0, 0, 255);
	std::vector<cv::Vec4i> lines;
	cv::HoughLinesP(src, lines, 1, CV_PI / 180, 1);
	for (size_t i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		cv::line(src, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255), 3, CV_AA);
	}
}

void slidingWindow(cv::Mat src, cv::Mat &tracking) {
	int windows_n_rows = 20;
	int windows_n_cols = 20;

	cv::Point points_left[16];
	cv::Point points_right[16];

	// Step of each window
	int StepSlide = 1;
	int left = 0;
	int right = 0;
	int sum_left = 0;
	int sum_right = 0;
	double mean_left = 0;
	double mean_right = 0;
	int lmeet = 0;
	int rmeet = 0;
	for (int row = 0; row <= src.rows - windows_n_rows; row += 20)
	{
		for (int col = 0; col <= src.cols - windows_n_cols; col += StepSlide)
		{
			cv::Rect windows(col, row, windows_n_rows, windows_n_cols);
			cv::Mat Roi = src(windows);

			if (countNonZero(Roi) != 0) {
				lmeet++;
				if (lmeet == 1) points_left[left] = cv::Point(col + 10, row);
				col += 40;
			}

			if (col >= src.cols - windows_n_cols) {
				if (lmeet != 2) {
					points_left[left] = cv::Point(0, row);
				}
				left++;
				lmeet = 0;
			}
		}
	}

	for (int row = 0; row <= src.rows - windows_n_rows; row += 20)
	{
		for (int col = src.cols - windows_n_cols; col >= 0; col -= StepSlide)
		{
			cv::Rect windows(col, row, windows_n_rows, windows_n_cols);
			cv::Mat Roi = src(windows);

			if (countNonZero(Roi) != 0) {
				rmeet++;
				if (rmeet == 1) points_right[right] = cv::Point(col - 10, row);
				col -= 40;
			}

			if (col <= 0) {
				if (rmeet != 2) {
					points_right[right] = cv::Point(src.cols, row);
				}
				right++;
				rmeet = 0;
			}
		}
	}
	for (int i = 0; i < 16; i++) {
		cv::circle(tracking, points_left[i], 4, cv::Scalar(255, 0, 0), 4);
	}

	for (int i = 0; i < 16; i++) {
		cv::circle(tracking, points_right[i], 4, cv::Scalar(0, 255, 0), 4);
	}
}



