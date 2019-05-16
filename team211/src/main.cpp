#include <opencv2/opencv.hpp>
#include <ros/ros.h>
#include <ros/master.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <iostream>
#include <string>
#include <chrono>
using namespace cv;
using namespace std;
using namespace std::chrono;

bool STREAM = true;

int minThreshold[3] = { 0, 0, 160 };
int maxThreshold[3] = { 180, 44, 255 };
int erosionSize = 0;
int delta = 20;
int point_max = 16;

static int BIRDVIEW_WIDTH = 240;
static int BIRDVIEW_HEIGHT = 320;
static int VERTICAL;
static int HORIZONTAL;
int skyLine = 85;
int isShadow = 0;

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
	int k = 0;
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

void findNearest_first_left(Point* list, int start, int &one, int &two) {	
	int j = 0;
	for (int i = start+1; i < 16; i++) {
		if (list[i].x != 0) {
			if (j == 0) one = list[i].x;
			else if (j == 1) two = list[i].x;
			j++;
		}
		if (j == 2) break;
	}	
}

void findNearest_first_right(Point* list, int start, int &one, int &two) {
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

void findNearest_last_left(Point* list, int end, int &one, int &two) {
	int j = 0;
	for (int i = end-1; i >= 0; i--) {
		if (list[i].x != 0) {
			if (j == 0) one = list[i].x;
			else if (j == 1) two = list[i].x;
			j++;
		}
		if (j == 2) break;
	}
} 

void findNearest_last_right(Point* list, int end, int &one, int &two) {
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

void findNearest_middle_left(Point* list, int pos, int &one, int &two, int &count) {
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

void findNearest_middle_right(Point* list, int pos, int &one, int &two, int &count) {
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

void nearestX_left(Point* list, int* position, int num) {
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
				int liuliu = abs(one - two)/(count+1);				
				if (one < two) {
					for (int k = 0; k < count; k++) {
						list[position[i + k]].x = one + (k+1)*liuliu;
					}
				}
				else if (one > two) {
					for (int k = 0; k < count; k++) {
						list[position[i+k]].x = one - (k + 1)*liuliu;
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

void nearestX_right(Point* list, int* position, int num, int max) {

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
	int lmeet = 0;
	int rmeet = 0;
	for (int row = 0; row <= src.rows - windows_n_rows; row += 20)
	{
		for (int col = 0; col <= src.cols - windows_n_cols; col += StepSlide)
		{
			Rect windows(col, row, windows_n_rows, windows_n_cols);
			Mat Roi = src(windows);

			if (countNonZero(Roi) != 0) {
				lmeet++;
				if (lmeet == 1) points_left[left] = Point(col + 10, row);
				col += 40;
			}

			if (col >= src.cols - windows_n_cols) {
				if (lmeet != 2) {
					points_left[left] = Point(0, row);
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
			Rect windows(col, row, windows_n_rows, windows_n_cols);
			Mat Roi = src(windows);

			if (countNonZero(Roi) != 0) {
				rmeet++;
				if (rmeet == 1) points_right[right] = Point(col - 10, row);
				col -= 40;
			}

			if (col <= 0) {
				if (rmeet != 2) {
					points_right[right] = Point(src.cols, row);
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
		circle(tracking, points_left[i], 4, Scalar(255, 0, 0), 4);
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
		circle(tracking, points_right[i], 4, Scalar(0, 255, 0), 4);
	}
}

/*
Mat LaneInShadow(Mat img) {
Mat imgShadow;
inRange(img, Scalar(minThreshold[0], minThreshold[1], minThreshold[2]),
Scalar(maxThreshold[0], maxThreshold[1], maxThreshold[2]),
imgShadow);
}
*/

Mat detectLane(Mat img) {
	/*
	int k = waitKey(0);
	if (k == 32) {
	isShadow = 1;
	}
	else if (k == 27) {
	isShadow = 0;
	}
	*/
	//imshow("HSV1", img);
	cvtColor(img, img, CV_BGR2HSV);
	//imshow("HSV", img);
	//if (isShadow == 1) Mat imgShadow = LaneInShadow(img);
	Mat imgThresholded;
    auto start = high_resolution_clock::now(); 
	inRange(img, Scalar(minThreshold[0], minThreshold[1], minThreshold[2]),
		Scalar(maxThreshold[0], maxThreshold[1], maxThreshold[2]),
		imgThresholded);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout<< "Inrange time elapse "<< duration.count()<<endl;

	Mat element = getStructuringElement(MORPH_CROSS,
		Size(2 * erosionSize + 1, 2 * erosionSize + 1),
		Point(erosionSize, erosionSize));
	erode(imgThresholded, imgThresholded, element);
	//imshow("Threshold", imgThresholded);
	Mat imgBirdseye = birdViewTranform(imgThresholded);
	//imshow("BirdsEye", imgBirdseye);
    start = high_resolution_clock::now();
	fillLane(imgBirdseye);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    cout << "FillLane time elapse "<< duration.count()<<endl;

	//imshow("Fill", imgBirdseye);
	//histogramCalculate(imgBirdseye);
	Mat tracking = Mat::zeros(imgBirdseye.size(), CV_8UC3);
    start = high_resolution_clock::now();
	slidingWindow(imgBirdseye, tracking);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    cout<< "Sliding window time elapse "<< duration.count()<<endl;
	//imshow("Tracking", tracking);
	return imgThresholded;
}

void autoCheck() {
	for (int idx = 222; idx <= 1000; idx++) {
		string name = to_string(idx);
		string savepath = "C:/Users/15122/Documents/Visual Studio 2015/Projects/Default/Default/image/101c" + name + ".jpg";
		Mat img;
		cout << idx << endl;
		img = imread(savepath);
		detectLane(img);
	}
}

void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
    cv_bridge::CvImagePtr cv_ptr;
    Mat out;
    static int frame_num = 0;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        cout << "===================== Frame "<<frame_num<<"======================"<<endl;
        auto start_app = high_resolution_clock::now();
        detectLane(cv_ptr->image);
        auto stop_app = high_resolution_clock::now();
        auto app_duration = duration_cast<microseconds>(stop_app - start_app);
        cout << "detectLane time elapse "<< app_duration.count()<<endl;
        frame_num ++;
        imshow("View", cv_ptr->image);
        waitKey(1);
        
    }
    catch (cv_bridge::Exception &e)
    {
        ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
    }
}

int main(int argc, char** argv)
{
	//Mat img = imread("C:/Users/15122/Documents/Visual Studio 2015/Projects/Default/Default/image/101c222.jpg");
	//Mat img = imread("C:/Users/15122/Documents/Visual Studio 2015/Projects/TestOpenCV/TestOpenCV/image/1.png");
	//Mat imgThresholded = detectLane(img);
    ros::init(argc,argv, "image_listener");
    cv::namedWindow("View");
    if (STREAM) {
        cv::startWindowThread();

        ros::NodeHandle nh;
        image_transport::ImageTransport it(nh);
        image_transport::Subscriber sub = it.subscribe("team211_image", 1, imageCallback);

        ros::spin();
    } 
    cv:: destroyAllWindows();
	autoCheck();
	return 0;
}
