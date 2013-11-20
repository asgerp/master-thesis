#include <stdio.h>
#include <iostream>
#include <time.h>
#include <boost/filesystem.hpp>

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;


void readme();
vector< Mat > getMatFromDir(string dir);
int detect(Mat obj, Mat scene);

const double NND_RATIO = 0.7;
const double PATTERN_SCALE = 22.0;


/** @function main */
int main( int argc, char** argv )
{
    /*if( argc != 3 )
     { readme(); return -1; }
     
     Mat img_scene = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
     
     vector< Mat > img_objs = getMatFromDir( argv[2]);
     
     cout << "found: " << img_objs.size() << " images" << endl;
     
     if( !img_scene.data )
     { cout<< " --(!) Error reading images " << endl; return -1; }
     for( int i = 0; i < img_objs.size(); i++ ) {
     detect(img_objs.at(i), img_scene);
     }
     */
    vector< Mat > img_objs = getMatFromDir( argv[1]);
    time_t start, end;
    int counter=0;
    
    
    VideoCapture cap(0);
    //320x240
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    
    cap.set(CV_CAP_PROP_CONVERT_RGB , false);
    cap.set(CV_CAP_PROP_FPS , 60);
    
    if (!cap.isOpened())
    { cout << "could not capture";
        return 0; }
    
    Mat frame;
    namedWindow("camera", 1);
    char key = 'a';
    time(&start);
    cout << "Starting" << endl;
    while(key != 27)
    {   cap.read( frame);
        detect(img_objs.at(0), frame);
        //        imshow("camera", frame);
        
        //##################
        //time at the end of 1 show, Stop the clock and show FPS
        time(&end);
        ++counter;
        cout <<"fps: "<< counter/ difftime(end,start) <<endl <<endl;
        cout << "FPS: " << cap.get(CV_CAP_PROP_FPS) << endl;
        //##################
        
        key = waitKey(3); }
    
    destroyAllWindows();
    return 0;
}

int detect(Mat img_object, Mat img_scene) {
    clock_t t1,t2;
    t1=clock();
    Mat gray;
    
    cvtColor(img_scene, gray, CV_BGR2GRAY);
    //-- Step 1: Detect the keypoints using FAST Detector
    FastFeatureDetector detector;
    
    vector<KeyPoint> keypoints_object, keypoints_scene;
    
    detector.detect( img_object, keypoints_object );
    detector.detect( gray, keypoints_scene );
    
    //-- Step 2: Calculate descriptors (feature vectors)
    FREAK extractor(true, true, PATTERN_SCALE, 4, vector<int>());
    
    Mat descriptors_object, descriptors_scene;
    
    extractor.compute( img_object, keypoints_object, descriptors_object );
    extractor.compute( img_scene, keypoints_scene, descriptors_scene );
    
    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    vector<vector< DMatch> > matches;
    
    if(descriptors_object.type()!=CV_32F) {
        descriptors_object.convertTo(descriptors_object, CV_32F);
    }
    if(descriptors_scene.type()!=CV_32F) {
        descriptors_scene.convertTo(descriptors_scene, CV_32F);
    }
    matcher.knnMatch(descriptors_object, descriptors_scene, matches, 2);
    
    
    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
    vector< DMatch > good_matches;
    
    for (size_t i = 0; i < matches.size(); ++i)
    {
        if (matches[i].size() < 2)
            continue;
        
        const DMatch &m1 = matches[i][0];
        const DMatch &m2 = matches[i][1];
        
        if(m1.distance <= NND_RATIO * m2.distance)
            good_matches.push_back(m1);
    }
    
    Mat img_matches;
    drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
                good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    //-- Localize the object
    vector<Point2f> obj;
    vector<Point2f> scene;
    for( int i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }
    cout << "good matches " << obj.size() << endl;
    if (good_matches.size() < 4) {
        cout << " OBJECT NOT FOUND " << endl;
        t2=clock();
        float diff ((float)t2-(float)t1);
        float seconds = diff / CLOCKS_PER_SEC;
        cout << "FAST/FREAK -> RANSAC took " << seconds << " SECONDS" << endl;
        
    } else {
        Mat H = findHomography( obj, scene, CV_RANSAC );
        
        //-- Get the corners from the image_1 ( the object to be "detected" )
        vector<Point2f> obj_corners(4);
        obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_object.cols, 0 );
        obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); obj_corners[3] = cvPoint( 0, img_object.rows );
        vector<Point2f> scene_corners(4);
        
        perspectiveTransform( obj_corners, scene_corners, H);
        
        t2=clock();
        float diff ((float)t2-(float)t1);
        float seconds = diff / CLOCKS_PER_SEC;
        cout << "FAST/FREAK -> RANSAC took " << seconds << " SECONDS" << endl;
        
        //-- Draw lines between the corners (the mapped object in the scene - image_2 )
        line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
        line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
        line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
        line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
        
        //-- Show detected matches
        
        //imwrite("result.jpg", img_matches);
        //waitKey(0);
    }
    imshow( "camera", img_matches );
    return 0;
    
}


/** @function getMatFromDir */
vector< Mat > getMatFromDir(string dir)
{
    vector< Mat > files;
    try {
        if (exists(dir)){
            if (is_directory(dir)) {
                cout << dir << " is a directory containing:\n";
                
                typedef vector<path> vec;             // store paths,
                vec v;                                // so we can sort them later
                
                copy(directory_iterator(dir), directory_iterator(), back_inserter(v));
                
                sort(v.begin(), v.end());             // sort, since directory iteration
                // is not ordered on some file systems
                
                for (vec::const_iterator it (v.begin()); it != v.end(); ++it)
                {
                    cout << "extenstion is: " << extension(*it) << endl;
                    if(extension(*it) == ".jpg" || extension(*it) == ".png"){
                        string file_path = it->string();
                        files.push_back(imread(file_path, CV_LOAD_IMAGE_GRAYSCALE));
                        cout << "   " << *it << '\n';
                    }
                    
                }
            }
        }
        
    } catch (const filesystem_error& ex) {
        cout << ex.what() << '\n';
    }
    return files;
}

/** @function readme */
void readme()
{ cout << " Usage: ./open_cv_test <img scene> <folder train>" << endl; }