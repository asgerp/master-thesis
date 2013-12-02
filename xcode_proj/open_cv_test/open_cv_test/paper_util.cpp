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
vector< vector<KeyPoint> > PaperUtil::getKeyPointsFromTemplates(vector<Mat> templates){
    //Detect the keypoints using SURF Detector
    int minHessian = 500;
    SurfFeatureDetector detector( minHessian );
    vector< vector<KeyPoint> > key_points;
    for(vector<Mat>::iterator it = templates.begin(); it != templates.end(); ++it) {
        vector<KeyPoint> kp_object;
        detector.detect( *it, kp_object );
        key_points.push_back(kp_object);
    }
    return key_points;
}

vector< Mat > PaperUtil::getDescriptorsFromKP(vector<Mat> templates, vector< vector<KeyPoint> > key_points){
    SurfDescriptorExtractor extractor;

    vector< Mat > descriptor_objects;
    for(vector<int>::size_type i = 0; i != templates.size(); i++) {
        Mat des_object;
        extractor.compute( templates[i], key_points[i], des_object );
        descriptor_objects.push_back(des_object);
    }
    return descriptor_objects;
}

/** @function readme */
void PaperUtil::readme()
{ cout << " Usage: ./open_cv_test <img scene> <folder train>" << endl; }





