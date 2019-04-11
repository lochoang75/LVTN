#ifndef SIGN_DETECT_H
#define SIGN_DETECT_H 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <stdio.h>
using namespace cv;
using namespace std;

// Dectect sign 
// Return 0 if not 
// Else return size of sign
int detectSign(Mat frame, CascadeClassifier sign, String name);

/* Detect obstacle rock*/
vector<Point> detectRock(Mat frame, CascadeClassifier rock, String = "rock");

/* Detect obstacle stackbox*/
vector<Point> detectStackBox(Mat frame, CascadeClassifier stack_box, String = "stack box");

/* Detect obstacle stackbox*/
vector<Point> detectSingleBox(Mat frame, CascadeClassifier stack_box, String = "single box");

// Detect object
// return 0 if not
// Else return postition of object
vector<Point> detectOject(Mat frame, CascadeClassifier object, String name, double = 1.2);

#endif
