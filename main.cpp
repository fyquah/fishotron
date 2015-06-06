#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "rectangle.h"

using namespace cv;
using namespace std;

Mat src, src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

/// Function header
void thresh_callback(int, void* );

/** @function main */
int main( int argc, char** argv ) {
    /// Load source image and convert it to gray
    src = imread( argv[1], 1 );
    cv::resize(src, src, cv::Size(700, 500));

    /// Convert image to gray
    cvtColor(src, src_gray, CV_BGR2GRAY);

    /// Create Window
    char* source_window = "Source";
    namedWindow( source_window, CV_WINDOW_AUTOSIZE );

    createTrackbar("Threshold:", "Source", &thresh, max_thresh, thresh_callback );
    thresh_callback(0, 0);
    waitKey(0);
    return(0);
}

void thresh_callback(int, void* ){
    RotatedRect minRect;
    Mat drawing;
    Point2f rectPoints[4]; 
    
    if(!obtainRectangle(src_gray, thresh, minRect)) {
        cout << "Unable to obtain best rectangle!" << endl;
        return;
    } 
    minRect.points(rectPoints);
    cvtColor(src_gray, drawing, CV_GRAY2BGR);    
    for (int i = 0 ; i < 4 ; i++) {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        line(drawing, rectPoints[i], rectPoints[(i+1) % 4], color , 1, 8);
    }
    // Detected rectangle
    namedWindow("Detected rectangle", CV_WINDOW_AUTOSIZE );
    imshow("Detected rectangle", drawing );
}