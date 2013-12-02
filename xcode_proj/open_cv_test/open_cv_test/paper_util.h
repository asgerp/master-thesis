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

class PaperUtil {
    public:
        static vector< Mat > getMatFromDir(string dir);
        static vector< vector<KeyPoint> >getKeyPointsFromTemplates(vector< Mat >);
        static vector< Mat > getDescriptorsFromKP(vector<Mat>, vector< vector<KeyPoint> >);
        static void readme();
};


#endif /* defined(__open_cv_test__paper_util__) */
