#include "detect_lane.h"
#include "xf_inrange_config.h"
#include "xf_erosion_config.h"
#include <chrono>
using namespace std::chrono;
using namespace std;

void detectLane(cv::Mat &img) {
	static xf::Mat<XF_8UC1,H,W,NPIX> imgInput(img.rows,img.cols);
	cv::cvtColor(img, img, CV_BGR2GRAY);
	imgInput.copyTo(img.data);
	xf::Mat<XF_8UC1,H,W,NPIX> img_gray(img.rows,img.cols);
	xf::Mat<XF_8UC1,H,W,NPIX> img_gray1(img.rows,img.cols);
	short int lower_thresh=180;
	short int upper_thresh=254;
	//auto start = high_resolution_clock::now();
	inrange_accel(imgInput, lower_thresh, upper_thresh, img_gray);
	//auto stop = high_resolution_clock::now();
	//auto duration = duration_cast<microseconds>(stop - start);
	//cout<< "Inrange time elapse "<<duration.count()<<endl;
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

void findNearest_first_left(cv::Point* list, int start, int &one, int &two) {
	int j = 0;
	for (int i = start + 1; i < 16; i++) {
		if (list[i].x != 0) {
			if (j == 0) one = list[i].x;
			else if (j == 1) two = list[i].x;
			j++;
		}
		if (j == 2) break;
	}
}

void findNearest_first_right(cv::Point* list, int start, int &one, int &two) {
	int j = 0;
	for (int i = start + 1; i < 16; i++) {
		if (list[i].x < 240) {
			if (j == 0) one = list[i].x;
			else if (j == 1) two = list[i].x;
			j++;
		}
		if (j == 2) break;
	}
}

void findNearest_last_left(cv::Point* list, int end, int &one, int &two) {
	int j = 0;
	for (int i = end - 1; i >= 0; i--) {
		if (list[i].x != 0) {
			if (j == 0) one = list[i].x;
			else if (j == 1) two = list[i].x;
			j++;
		}
		if (j == 2) break;
	}
}

void findNearest_last_right(cv::Point* list, int end, int &one, int &two) {
	int j = 0;
	for (int i = end - 1; i >= 0; i--) {
		if (list[i].x < 240) {
			if (j == 0) one = list[i].x;
			else if (j == 1) two = list[i].x;
			j++;
		}
		if (j == 2) break;
	}
}

void findNearest_middle_left(cv::Point* list, int pos, int &one, int &two, int &count) {
	int j = 0;
	count = 0;
	for (int i = pos - 1; i >= 0; i--) {
		if (list[i].x != 0) {
			if (j == 0) one = list[i].x;
			j++;
		}
		if (j == 1) break;
	}
	j = 0;
	for (int i = pos + 1; i < 16; i++) {
		if (list[i].x != 0) {
			count++;
			if (j == 0) two = list[i].x;
			j++;
		}
		if (j == 1) break;
	}
}

void findNearest_middle_right(cv::Point* list, int pos, int &one, int &two, int &count) {
	int j = 0;
	count = 0;
	for (int i = pos - 1; i >= 0; i--) {
		if (list[i].x < 240) {
			if (j == 0) one = list[i].x;
			j++;
		}
		if (j == 1) break;
	}
	j = 0;
	for (int i = pos + 1; i < 16; i++) {
		if (list[i].x < 240) {
			count++;
			if (j == 0) two = list[i].x;
			j++;
		}
		if (j == 1) break;
	}
}

void nearestX_left(cv::Point* list, int* position, int num) {
	int i = 0;
	int valid = 15 - num;
	while (position[i] >= 0 && position[i] <= 15) {
		if (position[i] == 0) {
			if (valid >= 2) {
				int one = 0;
				int two = 0;
				findNearest_first_left(list, 0, one, two);
				int liuliu = one - two;
				if (liuliu < 0) {
					list[0].x = one - abs(liuliu);
				}
				else if (liuliu > 0) {
					list[0].x = one + liuliu;
				}
				else list[0].x = one;
			}
			else if (valid == 1) {
				int one = 0;
				int two = 0;
				findNearest_first_left(list, 0, one, two);
				list[0].x = one;
			}
			else {
				list[0].x = 0;
			}
		}
		else if (position[i] == 15) {
			if (valid >= 2) {
				int one = 0;
				int two = 0;
				findNearest_last_left(list, 15, one, two);
				int liuliu = one - two;
				if (liuliu < 0) {
					list[15].x = one - abs(liuliu);
				}
				else if (liuliu > 0) {
					list[15].x = one + liuliu;
				}
				else list[15].x = one;
			}
			else if (valid == 1) {
				int one = 0;
				int two = 0;
				findNearest_last_left(list, 0, one, two);
				list[0].x = one;
			}
			else {
				list[0].x = 0;
			}
		}
		else {
			if (valid >= 2) {
				int one = 0;
				int two = 0;
				int count = 0;
				findNearest_middle_left(list, position[i], one, two, count);
				int liuliu = abs(one - two) / (count + 1);
				if (one < two) {
					for (int k = 0; k < count; k++) {
						list[position[i + k]].x = one + (k + 1)*liuliu;
					}
				}
				else if (one > two) {
					for (int k = 0; k < count; k++) {
						list[position[i + k]].x = one - (k + 1)*liuliu;
					}
				}
				else {
					for (int k = 0; k < count; k++) {
						list[position[i + k]].x = one;
					}
				}
				if (count != 0) {
					i += (count - 1);
				}
			}
			else if (valid == 1) {
				int count = 0;
				int one = 0;
				int two = 0;
				findNearest_middle_left(list, position[i], one, two, count);
				list[0].x = one;
			}
			else {
				list[0].x = 0;
			}
		}
		i++;
	}
}

void nearestX_right(cv::Point* list, int* position, int num, int max) {

	int i = 0;
	int valid = 15 - num;
	while (position[i] >= 0 && position[i] <= 15) {
		if (position[i] == 0) {
			if (valid >= 2) {
				int one = 0;
				int two = 0;
				findNearest_first_right(list, 0, one, two);
				int liuliu = one - two;
				if (liuliu < 0) {
					list[0].x = one - abs(liuliu);
				}
				else if (liuliu > 0) {
					list[0].x = one + liuliu;
				}
				else list[0].x = one;
			}
			else if (valid == 1) {
				int one = 0;
				int two = 0;
				findNearest_first_right(list, 0, one, two);
				list[0].x = one;
			}
			else {
				list[0].x = max;
			}
		}
		else if (position[i] == 15) {
			if (valid >= 2) {
				int one = 0;
				int two = 0;
				findNearest_last_right(list, 15, one, two);
				int liuliu = one - two;
				if (liuliu < 0) {
					list[15].x = one - abs(liuliu);
				}
				else if (liuliu > 0) {
					list[15].x = one + liuliu;
				}
				else list[15].x = one;
			}
			else if (valid == 1) {
				int one = 0;
				int two = 0;
				findNearest_last_right(list, 0, one, two);
				list[0].x = one;
			}
			else {
				list[0].x = max;
			}
		}
		else {
			if (valid >= 2) {
				int one = 0;
				int two = 0;
				int count = 0;
				findNearest_middle_right(list, position[i], one, two, count);
				int liuliu = abs(one - two) / (count + 1);
				if (one < two) {
					for (int k = 0; k < count; k++) {
						list[position[i + k]].x = one + (k + 1)*liuliu;
					}
				}
				else if (one > two) {
					for (int k = 0; k < count; k++) {
						list[position[i + k]].x = one - (k + 1)*liuliu;
					}
				}
				else {
					for (int k = 0; k < count; k++) {
						list[position[i + k]].x = one;
					}
				}
				if (count != 0) {
					i += (count - 1);
				}
			}
			else if (valid == 1) {
				int count = 0;
				int one = 0;
				int two = 0;
				findNearest_middle_right(list, position[i], one, two, count);
				list[0].x = one;
			}
			else {
				list[0].x = max;
			}
		}
		i++;
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
		int position_left[16];
		int position_right[16];
		int left_pos = 0;
		int right_pos = 0;
		int bef_l = 0;
		int aft_l = 0;
		int flag_l = 0;
		int bef_r = 0;
		int aft_r = 0;
		int flag_r = 0;
		for (int i = 0; i < 16; i++) {
			if (points_left[i].x > 0) {
				if (flag_l > 0) {
					aft_l = points_left[i].x;
					if (bef_l - aft_l > 10) {
						points_left[i].x = bef_l - 5;
					}
					else if (bef_l - aft_l < -10) {
						points_left[i].x = bef_l + 5;
					}
				}
				bef_l = points_left[i].x;
				flag_l++;
			}
		}

		if (points_left[15].x == 0) {
			position_left[left_pos] = 15;
			left_pos++;
		}
		for (int i = 0; i < 15; i++) {
			if (points_left[i].x == 0) {
				position_left[left_pos] = i;
				left_pos++;
			}
		}

		nearestX_left(points_left, position_left, left_pos);

		for (int i = 0; i < 16; i++) {
			cv::circle(tracking, points_left[i], 4, cv::Scalar(255, 0, 0), 4);
		}

		for (int i = 0; i < 16; i++) {
			if (points_right[i].x < tracking.size().width) {
				if (flag_r > 0) {
					aft_r = points_right[i].x;
					if (bef_r - aft_r > 10) {
						points_right[i].x = bef_r - 5;
					}
					else if (bef_r - aft_r < -10) {
						points_right[i].x = bef_r + 5;
					}
				}
				bef_r = points_right[i].x;
				flag_r++;
			}
		}

		if (points_right[15].x == tracking.size().width) {
			position_right[right_pos] = 15;
			right_pos++;
		}
		for (int i = 0; i < 15; i++) {
			if (points_right[i].x == tracking.size().width) {
				position_right[right_pos] = i;
				right_pos++;
			}
		}

		nearestX_right(points_right, position_right, right_pos, tracking.size().width);

		for (int i = 0; i < 16; i++) {
			cv::circle(tracking, points_right[i], 4, cv::Scalar(0, 255, 0), 4);
		}
}



