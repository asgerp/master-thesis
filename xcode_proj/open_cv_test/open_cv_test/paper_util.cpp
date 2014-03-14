//
//  util.cpp
//  open_cv_test
//
//  Created by Asger Pedersen on 02/12/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
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
MarkerInfo PaperUtil::getMatFromDir(string dir)
{
    vector< Mat > files;
    vector< string > fNames;
    MarkerInfo mi;
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
                        fNames.push_back(file_path);
                    }
                    
                }
            }
        }
        
    } catch (const filesystem_error& ex) {
        cout << ex.what() << '\n';
    }
    mi.imageData = files;
    mi.fNames = fNames;
    return mi;
}
vector< vector<KeyPoint> > PaperUtil::getKeyPointsFromTemplates(vector<Mat> templates, int minHessian, int nOctaves, int nOctavesLayer){

    SurfFeatureDetector detector( minHessian, nOctaves, nOctavesLayer );
    
    vector< vector<KeyPoint> > key_points;
    for(vector<int>::size_type i = 0; i != templates.size(); i++) {
    //for(vector<Mat>::iterator it = templates.begin(); it != templates.end(); ++it) {
        Mat eq_template;
        
        //equalizeHist( templates[i], eq_template );
        
        vector<KeyPoint> kp_object;
        detector.detect( templates[i], kp_object );
        key_points.push_back(kp_object);
        std::cout << kp_object.size() << endl;
    }
    
    return key_points;
}

vector< Mat > PaperUtil::getDescriptorsFromKP(vector<Mat> templates, vector< vector<KeyPoint> > key_points){
    SurfDescriptorExtractor extractor;
//    FREAK extractor(true, true, 8.0, 4, vector<int>());

    vector< Mat > descriptor_objects;
    for(vector<int>::size_type i = 0; i != templates.size(); i++) {
        Mat des_object, eq_template;
        //equalizeHist( templates[i], eq_template );
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

void PaperUtil::foundMarker(vector< Point2f > marker_corners, vector<vector< Point2f > > found, size_t i){
    found.at(i) = (marker_corners);
}


Mat PaperUtil::alignCams(Mat logitech, Mat Kinect){
    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;
    
    SurfFeatureDetector detector( minHessian );
    
    std::vector<KeyPoint> keypoints_object, keypoints_scene;
    
    detector.detect( logitech, keypoints_object );
    detector.detect( Kinect, keypoints_scene );
    
    //-- Step 2: Calculate descriptors (feature vectors)
    SurfDescriptorExtractor extractor;
    
    Mat descriptors_object, descriptors_scene;
    
    extractor.compute( logitech, keypoints_object, descriptors_object );
    extractor.compute( Kinect, keypoints_scene, descriptors_scene );
    
    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );
    
    double max_dist = 0; double min_dist = 100;
    
    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_object.rows; i++ )
    { double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }
    
    //printf("-- Max dist : %f \n", max_dist );
    //printf("-- Min dist : %f \n", min_dist );
    
    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
    std::vector< DMatch > good_matches;
    
    for( int i = 0; i < descriptors_object.rows; i++ )
    { if( matches[i].distance < 3*min_dist )
    { good_matches.push_back( matches[i]); }
    }
    
    Mat img_matches;
    drawMatches( logitech, keypoints_object, Kinect, keypoints_scene,
                good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    
    
    //-- Localize the object from img_1 in img_2
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    
    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }
    
    Mat H = findHomography( obj, scene, RANSAC );
    
    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point(0,0); obj_corners[1] = Point( logitech.cols, 0 );
    obj_corners[2] = Point( logitech.cols, logitech.rows ); obj_corners[3] = Point( 0, logitech.rows );
    std::vector<Point2f> scene_corners(4);
    
    perspectiveTransform( obj_corners, scene_corners, H);
    
    
    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    Point2f offset( (float)logitech.cols, 0);
    line( img_matches, scene_corners[0] + offset, scene_corners[1] + offset, Scalar(0, 255, 0), 4 );
    line( img_matches, scene_corners[1] + offset, scene_corners[2] + offset, Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[2] + offset, scene_corners[3] + offset, Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[3] + offset, scene_corners[0] + offset, Scalar( 0, 255, 0), 4 );
    
    //-- Show detected matches
    imshow( "Good Matches & Object detection", img_matches );
    
    waitKey(0);
    
    return H;

}

// Goes through all found markers
bool PaperUtil::touchedMarker(vector< vector<Point2f> > found, Point2f pt) {
    double inside = -1;
    for (int i = 0; i < found.size(); i++) {
        inside = pointPolygonTest(found.at(i), pt, false);
        break;
    }
    if(inside >= 0){
        return true;
    }
    return false;
}




