#ifndef LANE_DETECT_H
#define LANE_DETECT_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <ros/ros.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include "polynomialregression.h"

#define NUMBER_ELEMENTS 31
#define NUMBER_COEFFS 3

using namespace std;
using namespace cv;

class DetectLane
{
public:
    DetectLane();
    ~DetectLane();

    void update(const Mat &src);
    
    vector<Point> getLeftLane();
    vector<Point> getRightLane();
    vector<Point> getMidLane();
    vector<Point> getDesireLine();
    
    vector<Point> getLeftLine();
    vector<Point> getRightLine();
    vector<Point> getMidLine();

    static int slideThickness;

    static int BIRDVIEW_WIDTH;
    static int BIRDVIEW_HEIGHT;

    static int VERTICAL;
    static int HORIZONTAL;

    //static int NUMBER_ELEMENTS;


    
    static Point null; // 
    int analyseLine(int pos, int range);
    
    bool turn = false;
    
    int turnRight();
    int turnLeft();
    int turnRight2();
    int turnLeft2();
    int turnRightLane();
    int turnLeftLane();
    int turnRightLane2();
    int turnLeftLane2();
private:
    Mat preProcess(const Mat &src);

    bool isShadow(Mat src);

    Mat morphological(const Mat &imgHSV);
    Mat birdViewTranform(const Mat &source);
    void fillLane(Mat &src);
    vector<Mat> splitLayer(const Mat &src, int dir = VERTICAL);
    
    vector<vector<Point> > centerRoadSide(const vector<Mat> &src, int dir = VERTICAL);
    bool fillRoadSide(const vector<Mat> &src);
    

    
    void detectLeftRight(const vector<vector<Point> > &points);
    
    void update_desireLine(const vector<vector<Point> > &points);
    void update_last5Line(const vector<Point> &points);

    void update_coeffs(const vector<double> &new_coeffs);

    Mat laneInShadow(const Mat &src);

    //int minThreshold[3] = {0, 0, 180};
    int maxThreshold[3] = {179, 30, 255};
    
    int minThreshold[3] = {0, 0, 200};
    
    //int minShadowTh[3] = {90, 43, 36};
    //int maxShadowTh[3] = {120, 81, 171};
    int minShadowTh[3] = {0, 20, 36};
    int maxShadowTh[3] = {255, 97, 171};
    //int minLaneInShadow[3] = {90, 43, 97};
    //int maxLaneInShadow[3] = {120, 80, 171};
    int minLaneInShadow[3] = {0, 20, 97};
    int maxLaneInShadow[3] = {255, 97, 171};
    int binaryThreshold = 180;

    int skyLine = 85;
    int shadowParam = 40;

    vector<Point> leftLane, rightLane, desireLine, leftLine, rightLine, midLine, midLane;
    vector<vector<Point> > last5desireLine, last5left, last5right;
    vector<double> last_leftLine_coeffs, last_rightLine_coeffs, last_midLine_coeffs;
    vector<double> left_coeffs, right_coeffs, mid_coeffs;
};


#endif
