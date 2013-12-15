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
#include <dispatch/dispatch.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"

#include "DetectPaper.h"
#include "paper_util.h"

using namespace cv;


#ifdef DEBUG
#define PAPER_DEBUG(x) do { \
if (DEBUG) { std::cerr << __func__ << " " << __LINE__ << " " << x << std::endl; } \
} while (0)
#else
#define PAPER_DEBUG(x)
#endif


int main(int argc, char** argv )
{
    PAPER_DEBUG("here");
    std::string path;
    if(argc != 2){
        path = "/Users/asger/Documents/Skole/Master Thesis/runfolder/objs";
        PaperUtil::readme();
    } else{
        path = argv[1];
    }
    PAPER_DEBUG(path);
    // init templates. calculate key points and descriptors for templates
    vector< Mat > templates = PaperUtil::getMatFromDir(path);
    vector< vector< KeyPoint > > template_kp = PaperUtil::getKeyPointsFromTemplates(templates);
    vector< Mat > template_descriptors = PaperUtil::getDescriptorsFromKP(templates, template_kp);
    
    // init detectors, extractors, and matchers
    int minHessian = 500;
    SurfFeatureDetector detector( minHessian );
    SurfDescriptorExtractor extractor;
    BFMatcher matcher;
    
    // init video capture
    VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    cap.set(CV_CAP_PROP_CONVERT_RGB , false);
    
    namedWindow("Good Matches");
    
    /*std::vector<Point2f> obj_corners(4);
     
     //Get the corners from the object
     obj_corners[0] = cvPoint(0,0);
     obj_corners[1] = cvPoint( object.cols, 0 );
     obj_corners[2] = cvPoint( object.cols, object.rows );
     obj_corners[3] = cvPoint( 0, object.rows );*/
    
    //time_t start, end;
    double fps_start, start, end;
    //time(&start);
    fps_start = PaperUtil::getWallTime();
    
    clock_t t1,t2;
    
    int counter=0;
    char key = 'a';
    int framecount = 0;
    while (key != 27)
    {
        Mat frame, eq_frame, image;
        cap >> frame;
        cvtColor(frame, eq_frame, CV_RGB2GRAY);
        equalizeHist(eq_frame, image);
        
        if (framecount < 5)
        {
            framecount++;
            ++counter;
            continue;
        }
        t1=clock();
        start = PaperUtil::getWallTime();

        Mat des_image;
        std::vector<KeyPoint> kp_image;
        
        detector.detect( image, kp_image );
        extractor.compute( image, kp_image, des_image );
    
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
            //     code
            // })
            // dispatch_async(aQueue, ^{
            dispatch_time_t disp_start;
            disp_start = dispatch_walltime(NULL, 0);
            std::vector<std::vector<DMatch > > matches;
            std::vector<DMatch > good_matches;
            std::vector<Point2f> obj;
            std::vector<Point2f> scene;
            std::vector<Point2f> scene_corners(4);
            Mat H;
            matcher.knnMatch(template_descriptors[i], des_image, matches, 4);
            PAPER_DEBUG(" matches: " << matches.size());
            for(int h = 0; h < min(des_image.rows-1,(int) matches.size()); h++) //THIS LOOP IS SENSITIVE TO SEGFAULTS
            {
                if((matches[h][0].distance < 0.8*(matches[h][1].distance)) && ((int) matches[h].size()<=4 && (int) matches[h].size()>0))
                {
                    good_matches.push_back(matches[h][0]);
                }
            }
            
            //Draw only "good" matches
            PAPER_DEBUG("good matches: " << good_matches.size());
            //drawMatches( object, kp_object, image, kp_image, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
            
            // TODO: check if good_matches form a rect
            if (good_matches.size() >= 4)
            {
                for( int j = 0; j < good_matches.size(); j++ )
                {
                    //Get the keypoints from the good matches
                    obj.push_back( template_kp[i][ good_matches[j].queryIdx ].pt );
                    scene.push_back( kp_image[ good_matches[j].trainIdx ].pt );
                }
                
                H = findHomography( obj, scene, CV_RANSAC );
                std::vector<Point2f> obj_corners(4);
                
                //Get the corners from the object
                obj_corners[0] = cvPoint(0,0);
                obj_corners[1] = cvPoint( templates[i].cols, 0 );
                obj_corners[2] = cvPoint( templates[i].cols, templates[i].rows );
                obj_corners[3] = cvPoint( 0, templates[i].rows );
                perspectiveTransform( obj_corners, scene_corners, H);

                //Draw lines between the corners (the mapped object in the scene image )
                if(PaperUtil::checkAnglesInVector(obj_corners) == true && isContourConvex(Mat(obj_corners))){
                    PaperUtil::drawLine(image, scene_corners);
                } 
                PAPER_DEBUG(difftime(disp_start, dispatch_walltime(NULL, 0)));
            }
        });
        //}
        /*dispatch_sync(aQueue, ^{
         
         });*/
        imshow( "Good Matches", image );
        //time(&end);
        end = PaperUtil::getWallTime();
        ++counter;
        PAPER_DEBUG("fps: "<< counter/ difftime(end,fps_start) <<std::endl);
        t2=clock();
        float diff ((float)t2-(float)t1);
        float seconds = diff / CLOCKS_PER_SEC;

        PAPER_DEBUG("SURF/SURF -> RANSAC took " << (end - start) << " SECONDS");
        
        
        key = waitKey(1);
    }
    return 0;
}


