//
//  util.h
//  open_cv_test
//
//  Created by Asger Pedersen on 02/12/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//
#include "opencv2/core/core.hpp"
#ifndef __open_cv_test__paper_util__
#define __open_cv_test__paper_util__

#include <iostream>

using namespace std;
using namespace cv;

struct MarkerInfo {
    vector<Mat> imageData;
    vector<string> fNames;
};

struct HomographyInfo {
    Mat homography;
    vector<Point2f> roi;
};

class PaperUtil {
public:
    static MarkerInfo getMatFromDir(string dir);
    static vector< vector<KeyPoint> >getKeyPointsFromTemplates(vector< Mat >, int minHessian, int nOctaves, int nOctavesLayers);
    static vector< Mat > getDescriptorsFromKP(vector<Mat>, vector< vector<KeyPoint> >);
    static void drawLine(Mat img, vector<Point2f> corners);
    static void readme();
    static double getWallTime();
    static bool checkAnglesInVector(vector<Point2f> v);
    static void foundMarker(vector<Point2f> marker_corners, vector< vector<Point2f> > found, size_t i);
    static bool touchedMarker(vector< vector<Point2f> > found, Point2f pt);
    static HomographyInfo alignCams(Mat logitech, Mat Kinect);
private:
    static double angle(Point2f pt1, Point2f pt2, Point2f pt0 );    
};


#endif /* defined(__open_cv_test__paper_util__) */
