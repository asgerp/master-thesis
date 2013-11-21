//
//  DetectPaper.h
//  open_cv_test
//
//  Created by mundane on 21/11/2013.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#ifndef __open_cv_test__DetectPaper__
#define __open_cv_test__DetectPaper__

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

vector< Point > findPaper(Mat image, int minThresh, int maxThresh, bool preProcess);
vector<Point > getBestContour(vector< vector< Point > >);
bool checkAnglesInVector(vector<Point> v);
double angle(Point p1, Point p2, Point p0);

#endif /* defined(__open_cv_test__DetectPaper__) */
