/* positionCalc.cpp
 * 
 * written by: Jonathon Miller
 * Permission to use: Apache 2 License
 * Library used: openCV4
 *
 * Takes manually set pixel values and maps the 
 * points to a 2d plane.
 * 
 * The program finds foreground objects and creates
 * a bounding box around the object. The program assumes
 * that objects are on the bottom of a 2d plane.
 * The bounding box's lowest point is then transformed
 * to the 2d plane. The out put is a rough real world
 * coordinate for foreground objects.
 * 
 * date: 5/7/21
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/video.hpp>
#include <opencv4/opencv2/calib3d.hpp>

using namespace cv;
using namespace std;

//vectors to be used for homography
vector<Point2d> imageP;
vector<Point2d> worldP;

//vectors to be used by object detection
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

//initallize all the Mat varibles
Mat H = Mat_<double>(3,3);
Mat position = Mat_<double>(3,1);
Mat positionNorm = Mat_<double>(3,1);
Mat frame, fgMask, fgMaskNoShadow, cap, objPix, invH;
        

int width, height, threshold_area , area;

//initallize colors to be used for visual aid
Scalar red = Scalar(0, 0, 255);
Scalar white = Scalar(255, 255, 255);


int main (int argc, char* argv[])
{   
    //these values represent pixel values of the cooresponding 3D points
    imageP.push_back(Point2d( 260 , 420 ));
    imageP.push_back(Point2d( 664 , 366 ));
    imageP.push_back(Point2d( 356 , 301 ));
    imageP.push_back(Point2d( 143 , 258 ));
    imageP.push_back(Point2d( 846 , 339 ));
    imageP.push_back(Point2d( 594 , 300 ));
    imageP.push_back(Point2d( 396 , 263 ));

    //coplanar 3D points, the z value is assumed to be 1
    //used for normilzation later
    worldP.push_back(Point2d( 5 , 5 ));
    worldP.push_back(Point2d( 10 , 5 ));
    worldP.push_back(Point2d( 10 , 10 ));
    worldP.push_back(Point2d( 10 , 15 ));
    worldP.push_back(Point2d( 15 , 5 ));
    worldP.push_back(Point2d( 15 , 10 ));
    worldP.push_back(Point2d( 15 , 15 ));

    // finds a 3x3 matrix releationship between imageP and worldP
    // used to find the position of detected objects
    H = findHomography(imageP, worldP);
    
    
    //set resolution of the camera
    width = 1280;
    height = 720;

    //sets the minimum area in pixels a foreground object has to take up
    threshold_area = 1000;

    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> pBackSub;
    
    //Calls the MOG2 background subtraction
    //comparing the current frame to the 200 previous frames
    //with a threshold value of 14
    pBackSub = createBackgroundSubtractorMOG2(200, 14);
    
    //delcares an openCV video capture
    VideoCapture cap;
    
    //device number and driver used for the device
    cap.open(0, CAP_V4L );
    
    
    //sets the resolution of the camera
    cap.set(3, width);
    cap.set(4, height);

    for(;;)
    { 
       
        //takes a frame from the camera and rotates 180 degrees 
        cap >> frame;
	flip(frame, frame, -1);
	
        //updates the background model
        pBackSub->apply(frame, fgMask);
    
        //eleminating shadows from fgMask
        inRange(fgMask, Scalar(255), Scalar(255), fgMaskNoShadow);
	
	//used to display the output of countrours later
        Mat drawing = Mat::zeros (fgMaskNoShadow.size(), CV_8UC3);
        
        //finds the outer most countours of foreground objects
        findContours(fgMaskNoShadow, contours, hierarchy, 
		     RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        
	//preps two varibles for use in the next loop
	vector<Rect> boundRect( contours.size() );
	vector<Point> centers( contours.size() );
	
        for (size_t i = 0; i<contours.size(); i++)
        {
	    
	    //limits the number of countours examined to a
	    //threshhold value
	    area = cv::contourArea(Mat(contours[i]));
	    if (area > threshold_area)
	    {
		//creates a red bounding box around each countour
            	boundRect[i] = boundingRect( contours[i] );
            	rectangle( frame, boundRect[i].tl(), boundRect[i].br(), 
			   red, 2 );
		
		//finds the bottom center of the bounding box
		centers[i].x = boundRect[i].x + (boundRect[i].width)/2;
		centers[i].y = boundRect[i].y + boundRect[i].height;
		
		//converts the bottom center pixel to a Mat varible
		Mat imgPix = (Mat_<double>(3,1) << centers[i].x, 
			      centers[i].y, 1);
		
		//finds the unnormilzed position value	
		position = H * imgPix;
		
		//divides position by the z term to 
		//normilze the z term to 1
		divide( position, position.at<double>(2,0), 
		        positionNorm);
		
		//output the position to the consel
		cout << positionNorm << endl << endl;
		
		//outputs the normilzed position to the frame
		putText( frame, (to_string(positionNorm.at<double>(0,0))
			 + " "+ to_string(positionNorm.at<double>(0,0))), 
			 (centers[i]), FONT_HERSHEY_SIMPLEX, 
			 .75, Scalar(255,255,255));
		//used to showcase what is part of the foreground
            	drawContours ( drawing, contours, (int)i, white, 1, 
				 4, hierarchy, 0 );
	    }
	    
        }
       
        //show the current frame, the fg masks, and countours
        imshow("Frame", frame);
        imshow("NoShadow", fgMaskNoShadow);
        imshow("Contours", drawing);
       
        //needed to display an image to the display
        waitKey(100);
	
    }
    
    return 0;
}