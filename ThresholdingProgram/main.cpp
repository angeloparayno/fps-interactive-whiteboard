#include "opencv2\highgui\highgui.hpp"
#include "cv.h"
#include "OpenNI.h"

#include <stdio.h>

using namespace std;
using namespace cv;
using namespace openni;

int main()
{
	//KINECT INITIALIZATION
	openni::Device device;  
	openni::VideoStream color;
	openni::VideoFrameRef colorFrame;

	openni::OpenNI::initialize();
	device.open(openni::ANY_DEVICE);
	color.create(device, openni::SENSOR_COLOR);
	color.start();

	//FRAMES
	Mat imgKinect;
	Mat imgHSV;
	Mat imgThresholded;

	//HSV TRACKBAR
	namedWindow("Control",CV_WINDOW_AUTOSIZE);			//create a window called "Control"
	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0; 
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;

	//Create trackbars in "Control" window
	cvCreateTrackbar("LowH", "Control", &iLowH, 360);	//Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 360);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255);	//Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255);	//Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	//STREAMS
	char ch = ' ';
	while (ch != 27)
	{
		//Capture RGB stream from Kinect
		color.readFrame(&colorFrame);
		const openni::RGB888Pixel* imageBuffer = (const openni::RGB888Pixel*)colorFrame.getData();
		imgKinect.create(colorFrame.getHeight(),colorFrame.getWidth(),CV_8UC3);
		memcpy(imgKinect.data, imageBuffer,3*colorFrame.getHeight()*colorFrame.getWidth()*sizeof(uint8_t));
		cv::cvtColor(imgKinect,imgKinect,CV_BGR2RGB); 

		//Convert the captured frame from BGR to HSV
		cvtColor(imgKinect, imgHSV, COLOR_BGR2HSV); 

		//Threshold the image
		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); 
      
		//morphological opening (removes small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

		//morphological closing (removes small holes from the foreground)
		//dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
		//erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

		imshow("Thresholded Image", imgThresholded);	//show the thresholded image
		imshow("Original", imgKinect);					//show the original image
	   
		ch = cv::waitKey(1);
	}
	return 0;
}

