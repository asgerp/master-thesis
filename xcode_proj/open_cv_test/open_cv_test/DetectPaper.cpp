//
//  DetectPaper.cpp
//  open_cv_test
//
//  Created by mundane on 21/11/2013.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#include "DetectPaper.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

// find paper (brightest square thing) corners in image
// returns vector of 4 points corresponding to corners
vector< Point > findPaper(Mat image, int minThresh, int maxThresh, bool preProcess){
    Mat output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    // do some image preprocessing (careful, will segfault if picture already grayscale)
    if (preProcess) {
        cvtColor( image, image, CV_BGR2GRAY );
        blur( image, image, Size(3,3) );
        equalizeHist(image, image);
    }
    
    /// Detect edges using Threshold
    threshold( image, output, minThresh, maxThresh, THRESH_BINARY );
    
    // adaptive threshold
    //adaptiveThreshold( image, output, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 25, 5.0);
    
    /// Find contours
    findContours( output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    // best contour is the largest trapezoid object left in the scene
    vector<Point> squares = getBestContour(contours);
    return squares;
}

// get best paper-looking object
vector<Point> getBestContour(vector<vector<Point> > contours) {
    vector<Point> approx, largest;
    vector<vector<Point > > squares;
    int largestSize;
    
    for (size_t i = 0; i < contours.size(); i++) {
        // approximate contour with accuracy proportional
        // to the contour perimeter
        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true); // kinda magical but works
        
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        int area = fabs(contourArea(Mat(approx)));
        if (approx.size() == 4 && area > 1000 && area > largestSize && isContourConvex(Mat(approx)))
        {
            
            largestSize = area;
            largest = approx;
            
            //if (checkAnglesInVector(approx)) {
            //    squares.push_back(largest);
            //}
        }
    }
    return largest;
}

// check if angle is kinda like in a square
bool checkAnglesInVector(vector<Point> v) {
    double maxCosine = 0;
    for (int j = 2; j < 5; j++) {
        double cosine = fabs(angle(v[j%4], v[j-2], v[j-1]));
        maxCosine = MAX(maxCosine, cosine);
    }
    if (maxCosine < 0.3)
        return true;
    else return false;
}
// helper function, determines angle between three points
double angle( Point pt1, Point pt2, Point pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

