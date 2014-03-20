//
//  main.cpp
//  image_grabber
//
//  Created by mundane on 20/03/2014.
//  Copyright (c) 2014 MNDN. All rights reserved.
//

#include <iostream>
#include <vector>
#include <map>

//opencv
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

int main(int argc, const char * argv[]) {
    
    string fname;
    if(argc != 2){
        cout << "Usage: ./image_grabber name_of_output_file.ext" << endl;
        return 0;
    } else{
        fname = argv[1];
    }

    VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);
    cap.set(CV_CAP_PROP_CONVERT_RGB , false);
    
    Mat frame, greyFrame, cropFrame;
    
    while (frame.empty()) {
        cap >> frame;
    }
    
    // camera helper values
    int xOffset = 320, yOffset = 0, width = 640, height = 960;
    Rect cropROI(xOffset, yOffset, width, height);
    cropFrame = frame(cropROI);
    cvtColor(cropFrame, greyFrame, CV_RGB2GRAY);
    flip(greyFrame, greyFrame, -1);
    imwrite(fname, greyFrame);
    return 0;
}

