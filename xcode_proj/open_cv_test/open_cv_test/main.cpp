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
#include <numeric>
#include <unistd.h>
#include <fstream>
#include <iostream>
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

// OCL
#include "opencv2/ocl/ocl.hpp"
#include "opencv2/nonfree/ocl.hpp"
using namespace cv::ocl;

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

// General init
clock_t start = clock();
clock_t currTime;
vector< vector< Point2f > > foundMarkers;
vector< int > activeMarkers;
vector< int > markerMissing;
bool touchLastIteration = false;

// OpenNI
xn::Context xnContext;
xn::DepthGenerator xnDepthGenerator;
xn::ImageGenerator xnImageGenerator;

int initOpenNI(const XnChar* fname) {
	XnStatus nRetVal = XN_STATUS_OK;
    
	// initialize context
	nRetVal = xnContext.InitFromXmlFile(fname);
	CHECK_RC(nRetVal, "InitFromXmlFile");
    
	// initialize depth generator
	nRetVal = xnContext.FindExistingNode(XN_NODE_TYPE_DEPTH, xnDepthGenerator);
	CHECK_RC(nRetVal, "FindExistingNode(XN_NODE_TYPE_DEPTH)");
    
	// initialize image generator
	nRetVal = xnContext.FindExistingNode(XN_NODE_TYPE_IMAGE, xnImageGenerator);
	CHECK_RC(nRetVal, "FindExistingNode(XN_NODE_TYPE_IMAGE)");
    
    XnBool isSupported = xnDepthGenerator.IsCapabilitySupported("AlternativeViewPoint");
    if(TRUE == isSupported) {
        XnStatus res = xnDepthGenerator.GetAlternativeViewPointCap().SetViewPoint(xnImageGenerator);
        if(XN_STATUS_OK != res) {
            printf("Getting and setting AlternativeViewPoint failed: %s\n", xnGetStatusString(res));
        }
    }
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


int main(int argc, char** argv ) {
    string path;
    
    if(argc != 2){
        PaperUtil::readme();
        return 0;
    } else{
        path = argv[1];
    }
    
    // generate logfile name
    char sbuffer[24];
    time_t date;
    struct tm * timeinfo;
    time(&date);
    timeinfo = localtime (&date);
    strftime(sbuffer,24,"%F-%H-%M-%S.txt",timeinfo);
    string logfile(sbuffer);
    ofstream out(logfile);
    
    // init detectors, extractors, and matchers
    int minHessian = 300;
    int nOctaves = 4;
    int nOctavesLayers = 4;
    
    // OCL SURF + BFmatcher + GPU init
    SURF_OCL oclSURF = SURF_OCL(minHessian,nOctaves,nOctavesLayers,true,0.01f,true);
    BruteForceMatcher_OCL<ocl::L2<float> > oclBFMatcher;
    ocl::DevicesInfo devices;
    ocl::getOpenCLDevices(devices);
    ocl::setDevice(devices[1]); // 0 = on-die gpu, 1 = peripheral gpu
    
    std::cout
    << "OCL device name: "
    << cv::ocl::Context::getContext()->getDeviceInfo().deviceName
    << std::endl;
    
    // init templates
    MarkerInfo markerInfo = PaperUtil::getMatFromDir(path);
    vector<Mat> templates = markerInfo.imageData;
    
    // OCL SURF, upload templates, create keypoint and descriptor oclMats
    vector< oclMat > oclTemplates, oclTemplateKeypoints, oclTemplateDescriptors;
    for (int i = 0; i < templates.size(); i++) {
        oclMat oclTemplate, oclTemplKeyp, oclTemplDesc;
        oclTemplate.upload(templates[i]);
        oclTemplates.push_back(oclTemplate);
        oclTemplateKeypoints.push_back(oclTemplKeyp);
        oclTemplateDescriptors.push_back(oclTemplDesc);
    }
    
    // OCL SURF, detect keypoints and extract descriptors
    for (int i = 0; i < oclTemplates.size(); i++) {
        oclSURF(oclTemplates[i],oclMat(),oclTemplateKeypoints[i],oclTemplateDescriptors[i]);
    }
    
    // Download template keypoints
    vector< vector< KeyPoint > > dlTemplKeypoints;
    vector< Mat > dlTemplDescriptors;
    for (int i = 0; i < oclTemplates.size(); i++) {
        vector< KeyPoint > dlTemplKp;
        oclSURF.downloadKeypoints(oclTemplateKeypoints[i], dlTemplKp);
        dlTemplKeypoints.push_back(dlTemplKp);
    }
    
    // init foundmarkers with empty data
    for (int j = 0; j<templates.size(); j++) {
        vector<Point2f> init_corners(4);
        //Get the corners from the object
        init_corners[0] = cvPoint( 0, 0 );
        init_corners[1] = cvPoint( 0, 0 );
        init_corners[2] = cvPoint( 0, 0 );
        init_corners[3] = cvPoint( 0, 0 );
        foundMarkers.push_back(init_corners);
        activeMarkers.push_back(0);
        markerMissing.push_back(0);
    }

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
    
    // touch-related mats
    Mat1s depth(480, 640); // 16 bit depth (in millimeters)
	Mat1b depth8(480, 640); // 8 bit depth
	Mat3b debug(480, 640); // debug visualization
	Mat1s foreground(640, 480);
	Mat1b touch(640, 480); // touch mask
	Mat1s background(480, 640);
	vector<Mat1s> buffer(nBackgroundTrain);
    //================= INIT KINECT VARIABLES END =============================//
    // init video capture
    __block VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);
    cap.set(CV_CAP_PROP_CONVERT_RGB , false);
    
    // wait between initialising cameras
    usleep(100000);
    
    //init openNi from config
	initOpenNI("niConfig.xml");
    
    // create background model (average depth)
    // save nBackgroundTrain frame in buffer and calculate average between them
    for (unsigned int i=0; i<nBackgroundTrain; i++) {
		xnContext.WaitAndUpdateAll();
		depth.data = (uchar*) xnDepthGenerator.GetDepthMap();
		buffer[i] = depth;
	}
	average(buffer, background);
    
    // fps calculation
    int frames = 0;
    double currentTime = 0, lastUpdateTime = 0, elapsedTime = 0;
    char key = 'a';
    
    // camera helper values
    int xOffset = 320, yOffset = 0, width = 640, height = 960;
    Rect cropROI(xOffset, yOffset, width, height);
    
    // grab frame from both cams and calc homography
    // logitech frame
    Mat logitechFrame, greyLogitechFrame, finalLogitechFrame;
    cap >> logitechFrame;
    Mat cropLogitechFrame = logitechFrame(cropROI);
    cvtColor(cropLogitechFrame, greyLogitechFrame, CV_RGB2GRAY);
    greyLogitechFrame.copyTo(finalLogitechFrame, Mat()); // deep copy needed for proper ocl upload
    
    // kinect frame
    xnContext.WaitAndUpdateAll();
    Mat3b kinectFrame(480,640);
    Mat greyKinectFrame;
    kinectFrame.data = (uchar*) xnImageGenerator.GetRGB24ImageMap();
    cvtColor(kinectFrame, greyKinectFrame, CV_RGB2GRAY);
    
    // get homography to align cameras, save location of paper
    cout << "trying to calculate kinect/logitech homography" << endl;
    HomographyInfo hi = PaperUtil::alignCams(finalLogitechFrame, greyKinectFrame);
    Mat homography = hi.homography;
    vector<Point2f> paperLoc = hi.roi;
    
    cout << "found kinect/logitech homography" << endl;

    __block bool doSURF = true;
    int maxMissingIterations = 2;

    while (key != 27)
    {
        // reads data from all nodes, eg. cameras(depth and rgb)
        while ( !(XN_STATUS_OK == xnContext.WaitAndUpdateAll())) {
            xnContext.WaitAndUpdateAll();
            usleep(100000); // sleep 100ms
        }
        
        // update 16 bit depth matrix
        depth.data = (uchar*) xnDepthGenerator.GetDepthMap();
        
        // set dispatch priority
        dispatch_queue_t aQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        
        // check if it's time to do a SURF detection pass
        if (doSURF) {
            dispatch_async(aQueue, ^{
                doSURF = false;
                
                // init frames used this iteration
                Mat frame, image, greyImage;
                while(frame.empty()) {
                    cap >> frame;
                    usleep(100000); // sleep 100ms
                }
                
                // crop
                Mat cropImage = frame(cropROI);
                cvtColor(cropImage, greyImage, CV_RGB2GRAY);
                
                // deep copy needed or OCL uploads ROI+rest of parent image mat
                greyImage.copyTo(image, Mat());
                
                // descriptor and keypoint from image
                vector<KeyPoint> kp_image, imageKp;
                
                // ocl SURF setup
                SURF_OCL oclSURF = SURF_OCL(minHessian,nOctaves,nOctavesLayers,true,0.01f,true);
                oclMat oclImage = oclMat(width,height,CV_8U); // move this to preallocate ?
                oclMat oclImageKeypoints = oclMat();
                oclMat oclImageDescriptors = oclMat();
                
                // ocl upload image to gpu mem and get image keypoints + descriptors
                oclImage.upload(image);
                oclSURF(oclImage,oclMat(),oclImageKeypoints,oclImageDescriptors);
                
                // download results from gpu
                vector<KeyPoint> dlImageKp;
                vector<float> dlImageDesc;
                oclSURF.downloadKeypoints(oclImageKeypoints, dlImageKp);
                oclSURF.downloadDescriptors(oclImageDescriptors, dlImageDesc);
                
                // give template matcher jobs to GCD
                //for (int i=0; i< templates.size();i++) {
                    //dispatch_async(aQueue, ^{
                    dispatch_apply(templates.size(), aQueue, ^(size_t i) {
                        vector<vector<DMatch > > matches;
                        vector<DMatch > good_matches;
                        vector<Point2f> obj;
                        vector<Point2f> scene;
                        vector<Point2f> scene_corners(4);
                        Mat H;
                        
                        // ocl BF matching
                        BruteForceMatcher_OCL<ocl::L2<float> > oclBFMatcher;
                        oclBFMatcher.knnMatch(oclTemplateDescriptors[i], oclImageDescriptors, matches, 2); // k < 2 bugged
                        
                        for(int h = 0; h < (dlImageDesc.size(),(int) matches.size()); h++) {
                            if((matches[h][0].distance < 0.85*(matches[h][1].distance)) && ((int) matches[h].size()<=2 && (int) matches[h].size()>0)) {
                                good_matches.push_back(matches[h][0]);
                            }
                        }
                        
                        if (good_matches.size() >= 4) {
                            
                            for( int j = 0; j < good_matches.size(); j++ ) {
                                //Get the keypoints from the good matches
                                obj.push_back( dlTemplKeypoints[i][ good_matches[j].queryIdx ].pt );
                                scene.push_back( dlImageKp[ good_matches[j].trainIdx ].pt );
                            }
                            H = findHomography( obj, scene, CV_RANSAC , 10.0 );
                            vector<Point2f> obj_corners(4);
                            
                            //Get the corners from the object
                            obj_corners[0] = cvPoint(0,0);
                            obj_corners[1] = cvPoint( templates[i].cols, 0 );
                            obj_corners[2] = cvPoint( templates[i].cols, templates[i].rows );
                            obj_corners[3] = cvPoint( 0, templates[i].rows );
                            perspectiveTransform( obj_corners, scene_corners, H);
                            
                            // Draw lines between the corners (the mapped object in the scene image )
                            // save the corners in a list if we think they are templates
                            int area = fabs(contourArea(Mat(scene_corners)));
                            
                            if(PaperUtil::checkAnglesInVector(scene_corners) == true && isContourConvex(Mat(scene_corners)) && 50000 > area && area > 5000){
                                PaperUtil::drawLine(image, scene_corners);
                                foundMarkers.at(i) = scene_corners;
                                activeMarkers[i] = 1;
                                markerMissing[i] = 0;
                            } else {
                                // increment up to N passes
                                if (markerMissing[i] < maxMissingIterations) {
                                    markerMissing[i] += 1;
                                }
                            }
                        }
                    });// dispatch block end
                //}
                doSURF = true;
            });// dispatch block end
        }
        // extract foreground by simple subtraction of very basic background model
		foreground = background - depth;
		// find touch mask by thresholding (points that are close to background = touch points)
		touch = (foreground > touchDepthMin) & (foreground < touchDepthMax);
        
		// extract ROI(region of interest)
        Rect roi = boundingRect(paperLoc);
		Mat touchRoi = touch(roi);
        // find touch points and return them
		vector< vector<Point2i> > contours;
		vector<Point2f> touchPoints;
        findContours(touchRoi, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, hi.roi[0]);
        
        for (int i=0; i<contours.size(); i++) {
            Mat contourMat(contours[i]);
			// find touch points by area thresholding
			if ( contourArea(contourMat) > touchMinArea ) {
				Scalar center = mean(contourMat);
				Point2f touchPoint(center[0], center[1]);
				touchPoints.push_back(touchPoint);
                vector<Point2f> floatContours(1);
                floatContours[0] = touchPoint;
                // if no touch last iteration we check for a new touch
                if (!touchLastIteration) {
                    for (int g = 0; g<foundMarkers.size(); g++) {
                        // transform perspective between kinect and logitech
                        vector<Point2f> transformedTouchpoint(floatContours.size());
                        perspectiveTransform(floatContours, transformedTouchpoint, homography.inv());
                        // test if transformed touch point is inside known marker location
                        //circle(image, transformedTouchpoint[0], 5, debugColor2, CV_FILLED);
                        double foundIt = pointPolygonTest(foundMarkers.at(g),transformedTouchpoint[0], false);
                        
                        if(foundIt > 0){
                            currTime = clock();
                            float diff = (float)currTime/CLOCKS_PER_SEC-(float)start/CLOCKS_PER_SEC;
                            out << markerInfo.fNames.at(g) << ", " << diff << endl;
                            out.flush();
                            cout << markerInfo.fNames.at(g) << " " << g << endl;
                            
                            
                        }
                    }
                }
                //cerr << touchPoint.x << "," << touchPoint.y << endl;
			}
        }
        
        // check if we touched anything
        if (touchPoints.size() > 0) {
            touchLastIteration = true;
        } else {
            touchLastIteration = false;
        }
        
		// draw debug frame
		depth.convertTo(depth8, CV_8U, 255 / debugFrameMaxDepth); // render depth to debug frame
		cvtColor(depth8, debug, CV_GRAY2BGR);
		debug.setTo(debugColor0, touch);  // touch mask
		rectangle(debug, roi, debugColor1, 2); // surface boundaries
        
        for (unsigned int i=0; i<touchPoints.size(); i++) { // touch points
			circle(debug, touchPoints[i], 5, debugColor2, CV_FILLED);
		}
        imshow("depth", debug);
        //imshow("debug", image);
        
        // calc how many active markers
        int sum = accumulate(activeMarkers.begin(), activeMarkers.end(), 0);
        
        // check if all markers are missing
        int sumMissMarkers = accumulate(markerMissing.begin(), markerMissing.end(),0);
        if (sumMissMarkers == maxMissingIterations*markerMissing.size()) {
            cout << "No markers!!" << endl;
            // reinit foundmarkers with empty data, reset active and missing
            vector< vector < Point2f > > reinitMarkers;
            for (int i = 0; i<templates.size(); i++) {
                vector<Point2f> init_corners(4);
                //Get the corners from the object
                init_corners[0] = cvPoint( 0, 0 );
                init_corners[1] = cvPoint( 0, 0 );
                init_corners[2] = cvPoint( 0, 0 );
                init_corners[3] = cvPoint( 0, 0 );
                reinitMarkers.push_back(init_corners);
                activeMarkers[i] = 0;
                markerMissing[i] = 0;
            }
            foundMarkers = reinitMarkers;
        }
        
        // output fps as last thing we do
        frames++;
        currentTime = getTickCount();
        elapsedTime = ( currentTime - lastUpdateTime ) * 1000.0 / getTickFrequency();
        
        if ( elapsedTime >= 1000.0 ) {
            cout << "fps: " << ((frames * 1000.0) / elapsedTime) <<  " active markers: " << sum << endl;
            frames = 0;
            lastUpdateTime = currentTime;
        }
        key = waitKey(1);
    }
    return 0;
}
