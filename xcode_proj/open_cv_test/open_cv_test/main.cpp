//
//  Other_test.cpp
//  open_cv_test
//
//  Created by Asger Pedersen on 10/11/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#include "Other_test.h"
#include <stdio.h>
#include <time.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"

#include "DetectPaper.h"

using namespace cv;

int main(int argc, char** argv )
{
    
    if(argc != 2){
        return -1;
    }
    Mat object = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
    
    if( !object.data )
    {
        std::cout<< "Error reading object " << std::endl;
        return -1;
    }
    
    //Detect the keypoints using SURF Detector
    int minHessian = 500;
    //FastFeatureDetector detector(15);
//    OrbFeatureDetector detector(1500,1.2,8,31,0,2,ORB::HARRIS_SCORE, 31);
    SurfFeatureDetector detector( minHessian );
    std::vector<KeyPoint> kp_object;
    
    detector.detect( object, kp_object );
    
    //Calculate descriptors (feature vectors)
    

    //-- Step 2: Calculate descriptors (feature vectors)
//  FREAK extractor(true, true, 22.0, 4, vector<int>());
//    OrbDescriptorExtractor extractor;
    SurfDescriptorExtractor extractor;
    Mat des_object;
    
    extractor.compute( object, kp_object, des_object );
    
    /*if(des_object.type()!=CV_32F) {
        des_object.convertTo(des_object, CV_32F);
    }*/
    std::cout << des_object.size() << std::endl;
    BFMatcher matcher;
    
    VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    cap.set(CV_CAP_PROP_CONVERT_RGB , false);
    
    namedWindow("Good Matches");
    
    std::vector<Point2f> obj_corners(4);
    
    //Get the corners from the object
    obj_corners[0] = cvPoint(0,0);
    obj_corners[1] = cvPoint( object.cols, 0 );
    obj_corners[2] = cvPoint( object.cols, object.rows );
    obj_corners[3] = cvPoint( 0, object.rows );
    time_t start, end;
    time(&start);
    clock_t t1,t2;

    int counter=0;
    char key = 'a';
    int framecount = 0;
    while (key != 27)
    {
        Mat frame, eq_frame, image;
        cap >> frame;
        cvtColor(frame, image, CV_RGB2GRAY);
        //equalizeHist(eq_frame, image);
        
        if (framecount < 5)
        {
            framecount++;
            ++counter;
            time(&end);
            continue;
        }
        t1=clock();
        Mat des_image, img_matches;
        std::vector<KeyPoint> kp_image;
        std::vector<vector<DMatch > > matches;
        std::vector<DMatch > good_matches;
        std::vector<Point2f> obj;
        std::vector<Point2f> scene;
        std::vector<Point2f> scene_corners(4);
        Mat H;
        
        

        detector.detect( image, kp_image );
        extractor.compute( image, kp_image, des_image );
        /*
        if(des_image.type()!=CV_32F) {
            des_image.convertTo(des_image, CV_32F);
        }
*/
        matcher.knnMatch(des_object, des_image, matches, 4);
        std::cout << " matches: " << matches.size() << std::endl;
        for(int i = 0; i < min(des_image.rows-1,(int) matches.size()); i++) //THIS LOOP IS SENSITIVE TO SEGFAULTS
        {
            if((matches[i][0].distance < 0.8*(matches[i][1].distance)) && ((int) matches[i].size()<=4 && (int) matches[i].size()>0))
            {
                good_matches.push_back(matches[i][0]);
            }
        }
        
        //Draw only "good" matches
        std::cout << "good matches: " << good_matches.size() << std::endl;
        drawMatches( object, kp_object, image, kp_image, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
        
        if (good_matches.size() >= 4)
        {
            for( int i = 0; i < good_matches.size(); i++ )
            {
                //Get the keypoints from the good matches
                obj.push_back( kp_object[ good_matches[i].queryIdx ].pt );
                scene.push_back( kp_image[ good_matches[i].trainIdx ].pt );
            }
            
            H = findHomography( obj, scene, CV_RANSAC );
            
            perspectiveTransform( obj_corners, scene_corners, H);
            
            //Draw lines between the corners (the mapped object in the scene image )
            line( img_matches, scene_corners[0] + Point2f( object.cols, 0), scene_corners[1] + Point2f( object.cols, 0), Scalar(0, 255, 0), 4 );
            line( img_matches, scene_corners[1] + Point2f( object.cols, 0), scene_corners[2] + Point2f( object.cols, 0), Scalar( 0, 255, 0), 4 );
            line( img_matches, scene_corners[2] + Point2f( object.cols, 0), scene_corners[3] + Point2f( object.cols, 0), Scalar( 0, 255, 0), 4 );
            line( img_matches, scene_corners[3] + Point2f( object.cols, 0), scene_corners[0] + Point2f( object.cols, 0), Scalar( 0, 255, 0), 4 );
        }
        
        // get paper coords and draw
        vector<Point> paper = findPaper(image, 127, 255, false);
        Scalar blue = Scalar( 255, 0, 0 );
        //std::cout << "paper points(lel):" << paper << std::endl;
        
        if (paper.size() > 0) { // only draw if detected (segfaults if you disregard)
            Point2f offset = obj_corners[1];
            line( img_matches, offset + Point2f(paper[0].x, paper[0].y), offset + Point2f(paper[1].x, paper[1].y), blue, 2);
            line( img_matches, offset + Point2f(paper[1].x, paper[1].y), offset + Point2f(paper[2].x, paper[2].y), blue, 2);
            line( img_matches, offset + Point2f(paper[2].x, paper[2].y), offset + Point2f(paper[3].x, paper[3].y), blue, 2);
            line( img_matches, offset + Point2f(paper[3].x, paper[3].y), offset + Point2f(paper[0].x, paper[0].y), blue, 2);
        }
        
        //Show detected matches
        imshow( "Good Matches", img_matches );
        time(&end);
        ++counter;
        std::cout <<"fps: "<< counter/ difftime(end,start) <<std::endl <<std::endl;
        t2=clock();
        float diff ((float)t2-(float)t1);
        float seconds = diff / CLOCKS_PER_SEC;
        std::cout << "SURF/SURF -> RANSAC took " << seconds << " SECONDS" << std::endl;

        key = waitKey(1);
    }
    return 0;
}
