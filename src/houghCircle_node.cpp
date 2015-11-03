#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>


using namespace cv;
using namespace std;

class MyPoint {
public:
    int x , y;
    MyPoint(int x , int y){
        this->x = x;
        this->y = y;
    }
};

class Image : public Mat{
public:
    int radiusForCheck;
    vector<MyPoint> outOfThreshold;
    Image(/*int radiusForCheck*/){
        //            this->radiusForCheck = radiusForCheck;
    }

};


bool houghCircle(Mat cannied , int &radius , vector<int> &xCenter , vector<int> &yCenter);

#define PI 3.1418

int main(int argc, char **argv){

    Mat gray_image ;
    Mat cou;
    Mat image , temp;

    std::string nodeName = "houghCircle_node";
    ros::init(argc, argv, nodeName);

    ros::NodeHandle nh;

    image_transport::ImageTransport it(nh);
    cv::VideoCapture cap(0);

    if(!cap.isOpened()){
        ROS_ERROR("camera can't open!!fuck!!");
    }

    image_transport::Publisher pub = it.advertise(nodeName, 1);
    ros::Rate loop_rate(100);
    cv::Mat frame ;

    frame = imread("/home/mohammad/Pictures/ball1.png");
    while (nh.ok()){
        vector<Vec3f> circles;
        cv::cvtColor(frame, gray_image, CV_RGB2GRAY);
        //  GaussianBlur(gray_image , gray_image , Size(1,1) , 2 , 2);
        Canny(gray_image, cou, 100, 120, 3);
        int radius ;
        std::vector<int> xCenter , yCenter;
        //        std::cout << "kjar" << std::endl;
        bool draw = houghCircle(cou,radius,xCenter,yCenter);
        //  cou = gray_image;
        //  Mat cannied;
        //  Canny(gray_image,cannied,70,80,10,9);
        cvtColor(cou , temp , CV_GRAY2BGR);



        //    HoughCircles(cou , circles , CV_HOUGH_GRADIENT,1, cou.rows/8, 80 , 50);
        //  for( size_t i = 0; i <circles.size() ; i++ ){
        //            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        //            int radius = cvRound(circles[i][2]);

        if(draw){
            for(int i = 0 ; i < xCenter.size() ; i++){
                Point center(xCenter.at(i),yCenter.at(i));
                std::cout << center.y<<"@@" << center.x <<std::endl;
                circle( temp, center, 3, Scalar(0,255,0), -1, 8, 0 );
                circle( temp, center, radius, Scalar(0,0,255), 2, 8, 0 );
                //        Vec3i c = circles[i];
                //        circle( temp, Point(c[0], c[1]), c[2], Scalar(0,0,255), 3, 8 ,0);
                //        circle( temp, Point(c[0], c[1]), 2, Scalar(0,255,0), 3, 8 ,0);
                //  }
            }
        }
        cvtColor(temp,image,CV_BGR2RGB);
        sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", image).toImageMsg();
        pub.publish(msg);
        ros::spinOnce();
        loop_rate.sleep();
    }
}

bool houghCircle(Mat cannied , int &radius , vector<int> &xCenter , vector<int> &yCenter){
    int min_r = 80 , max_r = 81/*cannied.cols/2*/;
    vector<Image> radiuses(max_r-min_r);
    for(int r = min_r ; r < max_r ; r++){
        Image& image = radiuses.at(r-min_r);
        image.radiusForCheck = r;
        int matrix[cannied.cols][cannied.rows];
        for(int x = 0 ; x < cannied.cols ; x++){
            for(int y = 0 ; y < cannied.rows ; y++){
                matrix[x][y] = 0;
            }
        }
        int pointsThreshold = 30;
        for(int x = 0 ; x < cannied.cols ; x++){
            for(int y = 0 ; y < cannied.rows ; y++){
                if((int)cannied.at<uchar>(y,x) == 255){
                    for(int k = 0 ; k < 360 ; k+=2){
                        int xCurrent = x + r*cos((double)(k*PI)/180.00);
                        int yCurrent = y + r*sin((double)(k*PI)/180.00);
                        if(xCurrent <= 0 || xCurrent >= cannied.cols-1
                                || yCurrent <= 0 || yCurrent >= cannied.rows-1
                                || matrix[xCurrent][yCurrent] >= pointsThreshold){
                            continue;
                        }
                        matrix[xCurrent][yCurrent]++;
                        if(matrix[xCurrent][yCurrent] >= pointsThreshold){
                            image.outOfThreshold.push_back(MyPoint(xCurrent,yCurrent));
                        }
                    }
                }
            }
        }
    }
    for(int i = 0; i < radiuses.size(); i++){
        Image image = radiuses.at(i);
        if(image.outOfThreshold.size()>0 && image.radiusForCheck >= min_r){
            radius = image.radiusForCheck;
            for(int j = 0 ; j < 1; j++){
                int x = image.outOfThreshold.at(j).x;
                int y = image.outOfThreshold.at(j).y;
                xCenter.push_back(x);
                yCenter.push_back(y);
            }
            std::cout << image.outOfThreshold.size() << "\n" ;
            return true;
        }
    }
    return false;
}
