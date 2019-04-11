#ifndef CARCONTROL_H
#define CARCONTROL_H
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <ros/ros.h>
#include "std_msgs/Float32.h"

#include <vector>
#include <math.h>

#include "lane_detect.h"

using namespace std;
using namespace cv;

class CarControl 
{
public:
    
    CarControl();
    ~CarControl();
    void driverCar(const vector<Point> &left, const vector<Point> &right, float velocity, float roadWidth);
    void drive_right();
    void drive_Right(const vector<Point> &right, float velocity);
    void drive_left();
    void drive_Left(const vector<Point> &left, float velocity);

    bool isTurnRight = false, turnRightFlag = false;
    
    bool isTurnLeft = false, turnLeftFlag = false; 


    bool isObtruction = 0;
    bool driveLeftLane = 0;
    bool driveRightLane = 0;
    //void set_Velocity(const std_msgs::Float32::ConstPtr& msg) {velocity = msg->data ;}

    //float get_Velocity();

private:
    float errorAngle(const Point &dst);
    
    ros::NodeHandle node_obj1;
    ros::NodeHandle node_obj2;
    //ros::NodeHandle node_obj3;
    
    ros::Publisher steer_publisher;
    ros::Publisher speed_publisher;
    //ros::Subscriber speed_subscriber;
    Point carPos;


    float laneWidth = 40;

    float velocity = 0;

    float minVelocity = 10;
    float maxVelocity = 40;
    float preError;

  
};


#endif
