//
//  util.cpp
//  open_cv_test
//
//  Created by Asger Pedersen on 02/12/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"

#include <boost/filesystem.hpp>

#include <stdio.h>
#include <iostream>
#include <sys/time.h>


#include "paper_util.h"

using namespace std;
using namespace cv;
using namespace boost::filesystem;

/** @function getMatFromDir */
vector< Mat > PaperUtil::getMatFromDir(string dir)
{
    vector< Mat > files;
    try {
        if (exists(dir)){
            if (is_directory(dir)) {
                
                typedef vector<path> vec;             // store paths,
                vec v;                                // so we can sort them later
                
                copy(directory_iterator(dir), directory_iterator(), back_inserter(v));
                
                sort(v.begin(), v.end());             // sort, since directory iteration
                // is not ordered on some file systems
                
                for (vec::const_iterator it (v.begin()); it != v.end(); ++it)
                {
                    if(extension(*it) == ".jpg" || extension(*it) == ".png"){
                        string file_path = it->string();
                        files.push_back(imread(file_path, CV_LOAD_IMAGE_GRAYSCALE));
                    }
                    
                }
            }
        }
        
    } catch (const filesystem_error& ex) {
        cout << ex.what() << '\n';
    }
    return files;
}
vector< vector<KeyPoint> > PaperUtil::getKeyPointsFromTemplates(vector<Mat> templates){
    int minHessian = 500;
    SurfFeatureDetector detector( minHessian );
//    FastFeatureDetector detector;
  
    vector< vector<KeyPoint> > key_points;
    for(vector<Mat>::iterator it = templates.begin(); it != templates.end(); ++it) {
        vector<KeyPoint> kp_object;
        detector.detect( *it, kp_object );
        key_points.push_back(kp_object);
        std::cout << kp_object.size() << endl;
    }
    
    return key_points;
}

vector< Mat > PaperUtil::getDescriptorsFromKP(vector<Mat> templates, vector< vector<KeyPoint> > key_points){
    SurfDescriptorExtractor extractor;
//    FREAK extractor(true, true, 22.0, 4, vector<int>());
    vector< Mat > descriptor_objects;
    for(vector<int>::size_type i = 0; i != templates.size(); i++) {
        Mat des_object;
        extractor.compute( templates[i], key_points[i], des_object );
        descriptor_objects.push_back(des_object);
        std::cout << des_object.size() << endl;
    }
    
    return descriptor_objects;
}

/** @function readme */
void PaperUtil::readme()
{ cout << " Usage: ./open_cv_test folder_with_templates/ " << endl; }

void PaperUtil::drawLine(Mat img, vector<Point2f> corners) {
    line( img, corners[0] , corners[1] , Scalar( 0, 255, 0), 4 );
    line( img, corners[1] , corners[2] , Scalar( 0, 255, 0), 4 );
    line( img, corners[2] , corners[3] , Scalar( 0, 255, 0), 4 );
    line( img, corners[3] , corners[0] , Scalar( 0, 255, 0), 4 );
    
}

double PaperUtil::getWallTime(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

// check if angle is kinda like in a square
bool PaperUtil::checkAnglesInVector(vector<Point2f> v) {
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
double PaperUtil::angle( Point2f pt1, Point2f pt2, Point2f pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}




