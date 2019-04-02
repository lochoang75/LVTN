#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>
using namespace cv;
using namespace std;

int minThreshold[3] = { 43, 0, 147 };
int maxThreshold[3] = { 104, 47, 255 };
int erosionSize = 0;
int delta = 20;
int point_max = 16;

static int BIRDVIEW_WIDTH = 240;
static int BIRDVIEW_HEIGHT = 320;
static int VERTICAL;
static int HORIZONTAL;
int skyLine = 85;

static int slideThickness;
vector<Point> leftLane, rightLane;
Point null = Point();


Mat birdViewTranform(const Mat &src)
{
	Point2f src_vertices[4];

	int width = src.size().width;
	int height = src.size().height;

	src_vertices[0] = Point(0, skyLine);
	src_vertices[1] = Point(width, skyLine);
	src_vertices[2] = Point(width, height);
	src_vertices[3] = Point(0, height);

	Point2f dst_vertices[4];
	dst_vertices[0] = Point(0, 0);
	dst_vertices[1] = Point(BIRDVIEW_WIDTH, 0);
	dst_vertices[2] = Point(BIRDVIEW_WIDTH - 105, BIRDVIEW_HEIGHT);
	dst_vertices[3] = Point(105, BIRDVIEW_HEIGHT);

	Mat M = getPerspectiveTransform(src_vertices, dst_vertices);

	Mat dst(BIRDVIEW_HEIGHT, BIRDVIEW_WIDTH, CV_8UC3);
	warpPerspective(src, dst, M, dst.size(), INTER_LINEAR, BORDER_CONSTANT);

	return dst;
}

void brightestValue(Mat img) {
	Mat gray;
	cvtColor(img, gray, CV_RGB2GRAY);
	double min, max;
	minMaxLoc(gray, &min, &max);
	cout << "Min: " << min << endl;
	cout << "Max: " << max << endl;
	imshow("Gray", gray);
	waitKey(0);
}

void fillLane(Mat &src)
{
	Scalar color[3];
	color[0] = Scalar(255, 0, 0);
	color[1] = Scalar(0, 255, 0);
	color[2] = Scalar(0, 0, 255);
	vector<Vec4i> lines;
	HoughLinesP(src, lines, 1, CV_PI / 180, 1);
	//cvtColor(src, src, CV_GRAY2RGB);
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(src, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 3, CV_AA);
	}
}

void histogramCalculate(Mat src) {
	int flag = 0;
	int* histogram1 = new int[src.size().width];
	int* histogram2 = new int[src.size().width];
	int* position1 = new int[src.size().width];;
	int* position2 = new int[src.size().width];;
	int pos1 = 0;
	int pos2 = 0;
	int max1 = 0;
	int max2 = 0;
	int pmax1 = -1;
	int pmax2 = -1;
	int count1 = 0;
	int count2 = 0;
	for (int i = 0; i < src.size().width; i++) {
		if (countNonZero(src.col(i)) != 0 && flag == 0) {
			histogram1[i] = countNonZero(src.col(i));
			if (histogram1[i] > max1) {
				max1 = histogram1[i];
				pmax1 = i;
			}
			position1[pos1] = i;
			pos1++;
			count1++;
			cout << histogram1[i] << " ";
			if (countNonZero(src.col(i + 1)) == 0) {
				flag = 1;
				cout << endl;
			}
		}
		else if (countNonZero(src.col(i)) != 0 && flag == 1) {
			histogram2[i] = countNonZero(src.col(i));
			if (histogram2[i] > max2) {
				max2 = histogram2[i];
				pmax2 = i;
			}
			position2[pos2] = i;
			pos2++;
			count2++;
			cout << histogram2[i] << " ";
			if (countNonZero(src.col(i + 1)) == 0) {
				flag = 0;
				break;
			}
		}
	}
	cout << endl;
	cout << "Position 1: ";
	int k=0;
	while (position1[k] > 0) {
		cout << position1[k] << " ";
		k++;
	}
	cout << endl;
	cout << "Max 1: " << max1 << ", Pmax1: " << pmax1 << endl;
	cout << "Position 2: ";
	int l = 0;
	while (position2[l] > 0) {
		cout << position2[l] << " ";
		l++;
	}
	cout << endl;
	cout << "Max 2: " << max2 << ", Pmax2: " << pmax2 << endl;

	circle(src, Point(pmax1, 20), 20, Scalar(255), 1);
	imshow("Circle", src);
}

int nearestX(Point* list, int position, double mean) {
	int i = position + 1;
	int prev = 0;
	int after = 0;
	int flag1 = 0;
	int flag2 = 0;
	while (i < point_max) {
		if (abs(list[i].x - mean) < delta) {
			after = list[i].x;
			flag1 = 1;
		}
		if (flag1 == 1) break;
		i++;
	}
	
	i = position - 1;
	while (i >= 0) {
		if (abs(list[i].x - mean) < delta) {
			prev = list[i].x;
			flag2 = 1;
		}
		if (flag2 == 1) break;
		i--;
	}
	
	if (after == 0) return prev;
	else if (prev == 0) return after;
	else if(after != 0 && prev != 0) return (int)((prev + after) / 2);
	else return mean;
}

void slidingWindow(Mat src, Mat &tracking) {
	int windows_n_rows = 20;
	int windows_n_cols = 20;

	Point points_left[16];
	Point points_right[16];

	// Step of each window
	int StepSlide = 1;
	int left = 0;
	int right = 0;
	int sum_left = 0;
	int sum_right = 0;
	double mean_left = 0;
	double mean_right = 0;
	for (int row = 0; row <= src.rows - windows_n_rows; row += 20)
	{
		for (int col = 0; col <= src.cols - windows_n_cols; col += StepSlide)
		{
			Rect windows(col, row, windows_n_rows, windows_n_cols);
			Mat Roi = src(windows);

			if (countNonZero(Roi) != 0) {			
				points_left[left] = Point(col + 10, row);
				left++;
				break;
			}
		}
	}

	for (int row = 0; row <= src.rows - windows_n_rows; row += 20)
	{
		for (int col = src.cols - windows_n_cols; col >= 0; col -= StepSlide)
		{
			Rect windows(col, row, windows_n_rows, windows_n_cols);
			Mat Roi = src(windows);

			if (countNonZero(Roi) != 0) {		
				points_right[right] = Point(col - 10, row);
				right++;
				break;
			}
		}
	}

	for (int i = 0; i < 16; i++) {
		sum_left += points_left[i].x;
	}
	mean_left = (double)sum_left / 16;

	for (int i = 0; i < 16; i++) {
		sum_right += points_right[i].x;
	}
	mean_right = (double)sum_right / 16;

	for (int i = 0; i < 16; i++) {
		if (abs(points_left[i].x - mean_left) > delta) {
			points_left[i].x = nearestX(points_left, i, mean_left);						
		}
		circle(tracking, points_left[i], 4, Scalar(255, 0, 0), 4);
	}

	for (int i = 0; i < 16; i++) {
		if (abs(points_right[i].x - mean_right) > delta) {
			points_right[i].x = nearestX(points_right, i, mean_right);
		}
		circle(tracking, points_right[i], 4, Scalar(0, 255, 0), 4);
	}
}

Mat detectLane(Mat img) {
	imshow("HSV1", img);
	cvtColor(img, img, CV_BGR2HSV);
	imshow("HSV", img);
	Mat imgThresholded;
	inRange(img, Scalar(minThreshold[0], minThreshold[1], minThreshold[2]),
		Scalar(maxThreshold[0], maxThreshold[1], maxThreshold[2]),
		imgThresholded);

	Mat element = getStructuringElement(MORPH_CROSS,
		Size(2 * erosionSize + 1, 2 * erosionSize + 1),
		Point(erosionSize, erosionSize));
	erode(imgThresholded, imgThresholded, element);
	imshow("Threshold", imgThresholded);
	Mat imgBirdseye = birdViewTranform(imgThresholded);
	imshow("BirdsEye", imgBirdseye);
	fillLane(imgBirdseye);
	imshow("Fill", imgBirdseye);
	//histogramCalculate(imgBirdseye);
	Mat tracking = Mat::zeros(imgBirdseye.size(), CV_8UC3);
	slidingWindow(imgBirdseye, tracking);
	imshow("Tracking", tracking);
	waitKey(0);
	return imgThresholded;
}

void autoCheck() {
	for (int idx = 7; idx <= 1000; idx++) {
		string name = to_string(idx);		
		string savepath = "C:/Users/15122/Documents/Visual Studio 2015/Projects/Default/Default/image/101c" + name + ".jpg";
		Mat img;
		cout << idx << endl;
		img = imread(savepath);
		detectLane(img);
	}
}

int main(int argc, char** argv)
{
	/*
	Mat img = imread("C:/Users/15122/Documents/Visual Studio 2015/Projects/Default/Default/image/101c300.jpg");
	Mat imgThresholded = detectLane(img);
	*/
	autoCheck();
	return 0;
}
