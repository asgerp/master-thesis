//
//  Other_test.cpp
//  open_cv_test
//
//  Created by Asger Pedersen on 10/11/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#include "Other_test.h"
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <map>
using namespace std;
//grand central dispatch
#include <dispatch/dispatch.h>

//opencv
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"


//openni
#include "ni/XnOpenNI.h"
#include "ni/XnCppWrapper.h"
using namespace xn;
#define CHECK_RC(rc, what)											\
if (rc != XN_STATUS_OK)											\
{																\
printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
return rc;													\
}

#include "DetectPaper.h"
#include "paper_util.h"

using namespace cv;

#ifdef DEBUG
#define PAPER_DEBUG(x) do { \
if (DEBUG) { cerr << "["<< __func__ << ":" << __LINE__ << "] " << x << endl; } \
} while (0)
#else
#define PAPER_DEBUG(x)
#endif

// OpenNI
xn::Context xnContext;
xn::DepthGenerator xnDepthGenerator;
xn::ImageGenerator xnImgeGenerator;

int initOpenNI(const XnChar* fname) {
	XnStatus nRetVal = XN_STATUS_OK;
    
	// initialize context
	nRetVal = xnContext.InitFromXmlFile(fname);
	CHECK_RC(nRetVal, "InitFromXmlFile");
    
	// initialize depth generator
	nRetVal = xnContext.FindExistingNode(XN_NODE_TYPE_DEPTH, xnDepthGenerator);
	CHECK_RC(nRetVal, "FindExistingNode(XN_NODE_TYPE_DEPTH)");
    
	// initialize image generator
	nRetVal = xnContext.FindExistingNode(XN_NODE_TYPE_IMAGE, xnImgeGenerator);
	CHECK_RC(nRetVal, "FindExistingNode(XN_NODE_TYPE_IMAGE)");
    
	return 0;
}

void average(vector<Mat1s>& frames, Mat1s& mean) {
	Mat1d acc(mean.size());
	Mat1d frame(mean.size());
    
	for (unsigned int i=0; i<frames.size(); i++) {
		frames[i].convertTo(frame, CV_64FC1);
		acc = acc + frame;
	}
    
	acc = acc / frames.size();
    
	acc.convertTo(mean, CV_16SC1);
}


int main(int argc, char** argv )
{
    string path;
    int noOfMarkers = 8;
    if(argc != 3){
        path = "/Users/asger/Documents/Skole/Master Thesis/runfolder/objs";
        PaperUtil::readme();
    } else{
        path = argv[1];
        istringstream iss( argv[2] );
        if(!(iss >> noOfMarkers)){
            noOfMarkers = 8;
        }
    }
    
    // init detectors, extractors, and matchers
    FlannBasedMatcher matcher;
    int minHessian = 100;
    int nOctaves = 4;
    int nOctavesLayers = 4;
    SurfFeatureDetector detector( minHessian , nOctaves, nOctavesLayers, true, false);
    SurfDescriptorExtractor extractor;

    // init templates. calculate key points and descriptors for templates/markers
    vector< Mat > templates = PaperUtil::getMatFromDir(path);
    vector< vector< KeyPoint > > template_kp = PaperUtil::getKeyPointsFromTemplates(templates, minHessian, nOctaves, nOctavesLayers);
    vector< Mat > template_descriptors = PaperUtil::getDescriptorsFromKP(templates, template_kp);

  	const char* windowName = "Debug";
    //================= INIT KINECT VARIABLES =============================//
    //init touch distance constants
	const unsigned int nBackgroundTrain = 30;
	const unsigned short touchDepthMin = 10;
	const unsigned short touchDepthMax = 20;
	const unsigned int touchMinArea = 50;
    
    // maximal distance (in millimeters) for 8 bit debug depth frame quantization
	const double debugFrameMaxDepth = 4000;
    const Scalar debugColor0(0,0,128);
	const Scalar debugColor1(255,0,0);
	const Scalar debugColor2(255,255,255);
    // area where we detect touch
	int xMin = 110;
	int xMax = 560;
	int yMin = 120;
	int yMax = 320;
    Mat1s depth(480, 640); // 16 bit depth (in millimeters)
	Mat1b depth8(480, 640); // 8 bit depth
	Mat3b rgb(480, 640); // 8 bit depth
    
	Mat3b debug(480, 640); // debug visualization
    
	Mat1s foreground(640, 480);
    //	Mat1b foreground8(640, 480);
    
	Mat1b touch(640, 480); // touch mask
    
	Mat1s background(480, 640);
	vector<Mat1s> buffer(nBackgroundTrain);
    
    //init openNi from config
	initOpenNI("niConfig.xml");
    
    
    namedWindow(windowName);
    // delete this stuf we dont need it
    // creates sliders
	createTrackbar("xMin", windowName, &xMin, 640);
	createTrackbar("xMax", windowName, &xMax, 640);
	createTrackbar("yMin", windowName, &yMin, 480);
	createTrackbar("yMax", windowName, &yMax, 480);

    
    // create background model (average depth)
    // save nBackgroundTrain frame in buffer and calculate average between them
	for (unsigned int i=0; i<nBackgroundTrain; i++) {
		xnContext.WaitAndUpdateAll();
		depth.data = (uchar*) xnDepthGenerator.GetDepthMap();
		buffer[i] = depth;
	}
	average(buffer, background);

/*    // init video capture
    VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    cap.set(CV_CAP_PROP_CONVERT_RGB , false);
  */

    
    // fps calculation
    double fps_start, start, end;
    fps_start = PaperUtil::getWallTime();
    
    clock_t t1,t2;
    
    int counter=0;
    char key = 'a';
    int framecount = 0;
    

    vector< vector<Point2f > > foundMarkers(templates.size());
    
    while (key != 27)
    {
        Rect rgbROI(xMin, yMin, xMax - xMin, yMax - yMin);
        // read available data
        // reads data from all nodes, eg. cameras(depth and rgb)
		xnContext.WaitAndUpdateAll();
        
        // update 16 bit depth matrix
		depth.data = (uchar*) xnDepthGenerator.GetDepthMap();
		//xnImgeGenerator.GetGrayscale8ImageMap()
		// update rgb image
		rgb.data = (uchar*) xnImgeGenerator.GetRGB24ImageMap(); // segmentation fault here
        Mat frame, eq_frame, image, image2, rgb2;

		cvtColor(rgb, eq_frame, CV_RGB2GRAY);
        
        
        //cap >> frame;
        //if background - frame < thresshold && we found all markers we are looking for
        // we only need to detect touches until the scene changes.
        
        
        // get frames from capture
        //cvtColor(frame, eq_frame, CV_RGB2GRAY);
//        equalizeHist(eq_frame, rgb2);
        
        image2 = eq_frame(rgbROI);
        resize(image2, image, Size(), 1.5, 1.5);

        
        t1=clock();
        start = PaperUtil::getWallTime();
        
        // descriptor image
        Mat des_image;
        vector<KeyPoint> kp_image;
        
        detector.detect( image, kp_image );
        extractor.compute( image, kp_image, des_image );

        // get dispatch queue
        dispatch_queue_t aQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);

        // OPTIMIZE THIS
        // function needs to know/ have access to:
        /**
         * templates, template_descritors[i]. des_image, matches
         * good_matches, template_kp, kp_image, obj, scene, scene_corners, image
         * 0.057101
         */
        //for(vector<int>::size_type i = 0; i != templates.size(); i++) {
        // the scope of the block is similar to inner for loops.
        // So the block should be able to read all varialbles
        
        dispatch_apply(templates.size(), aQueue, ^(size_t i) {
            
            vector<vector<DMatch > > matches;
            vector<DMatch > good_matches;
            vector<Point2f> obj;
            vector<Point2f> scene;
            vector<Point2f> scene_corners(4);
            Mat H;
            matcher.knnMatch(template_descriptors[i], des_image, matches, 2);
//            matcher.knnMatch(des_image, template_descriptors[i], matches,2);


            for(int h = 0; h < min(des_image.rows-1,(int) matches.size()); h++)
            {
                if((matches[h][0].distance < 0.8*(matches[h][1].distance)) && ((int) matches[h].size()<=2 && (int) matches[h].size()>0))
                {
                    good_matches.push_back(matches[h][0]);
                }
            }
//            PAPER_DEBUG(" matches: " << matches.size());
//            PAPER_DEBUG("good matches: " << good_matches.size());
            
            if (good_matches.size() >= 4)
            {
                for( int j = 0; j < good_matches.size(); j++ )
                {
                    //Get the keypoints from the good matches
                    obj.push_back( template_kp[i][ good_matches[j].queryIdx ].pt );
                    scene.push_back( kp_image[ good_matches[j].trainIdx ].pt );
                }
                
                H = findHomography( obj, scene, CV_RANSAC );
                vector<Point2f> obj_corners(4);
                
                //Get the corners from the object
                obj_corners[0] = cvPoint(0,0);
                obj_corners[1] = cvPoint( templates[i].cols, 0 );
                obj_corners[2] = cvPoint( templates[i].cols, templates[i].rows );
                obj_corners[3] = cvPoint( 0, templates[i].rows );
                perspectiveTransform( obj_corners, scene_corners, H);
                
                //Draw lines between the corners (the mapped object in the scene image )
                // count the number of markers found
                // save the corners in a list
                int area = fabs(contourArea(Mat(scene_corners)));
                
                if(PaperUtil::checkAnglesInVector(scene_corners) == true && area > 200){//&& isContourConvex(Mat(scene_corners)) && area > 5000){
                    PaperUtil::drawLine(image, scene_corners);
                    PAPER_DEBUG("FOUND " << area << " " << scene_corners[0] << " " << scene_corners[1] << " " << scene_corners[2] << " " << scene_corners[3]);
                }

            }
        });
        //} dispatch block end
        
        // extract foreground by simple subtraction of very basic background model
		foreground = background - depth;
        
		// find touch mask by thresholding (points that are close to background = touch points)
		touch = (foreground > touchDepthMin) & (foreground < touchDepthMax);


		// extract ROI(region of interest)
		Rect roi(xMin, yMin, xMax - xMin, yMax - yMin);
		Mat touchRoi = touch(roi);
        // find touch points and return them
		vector< vector<Point2i> > contours;
		vector<Point2f> touchPoints;
		findContours(touchRoi, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point2i(xMin, yMin));
		for (unsigned int i=0; i<contours.size(); i++) {
			Mat contourMat(contours[i]);
			// find touch points by area thresholding
			if ( contourArea(contourMat) > touchMinArea ) {
				Scalar center = mean(contourMat);
				Point2i touchPoint(center[0], center[1]);
				touchPoints.push_back(touchPoint);
                cerr << touchPoint.x << "," << touchPoint.y << endl;
			}
		}
        
        // comment out when going live
		// draw debug frame
		depth.convertTo(depth8, CV_8U, 255 / debugFrameMaxDepth); // render depth to debug frame
		cvtColor(depth8, debug, CV_GRAY2BGR);
		debug.setTo(debugColor0, touch);  // touch mask
		rectangle(debug, roi, debugColor1, 2); // surface boundaries
		for (unsigned int i=0; i<touchPoints.size(); i++) { // touch points
			circle(debug, touchPoints[i], 5, debugColor2, CV_FILLED);
		}
        imshow("depth", debug);
        imshow(windowName, image );
        end = PaperUtil::getWallTime();
        ++counter;
        //PAPER_DEBUG("fps: " << counter/ difftime(end,fps_start) <<endl);
        t2=clock();
        
        //PAPER_DEBUG("SURF/SURF -> RANSAC took " << (end - start) << " SECONDS");
        
        key = waitKey(1);
    }
    return 0;
}


