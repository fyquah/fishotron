#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src, src_gray, drawing;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

/// Function header
void thresh_callback(int, void* );

/** @function main */
int main( int argc, char** argv )
{
  /// Load source image and convert it to gray
  src = imread( argv[1], 1 );
  cv::resize(src, src, cv::Size(700, 500));

  /// Convert image to gray
  cvtColor(src, src_gray, CV_BGR2GRAY);

  /// Create Window
  char* source_window = "Source";
  namedWindow( source_window, CV_WINDOW_AUTOSIZE );


  imshow(source_window, src_gray);
  createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback );
  thresh_callback( 0, 0 );
  cout << "Done" << endl;
  waitKey(0);
  return(0);
}

/** @function thresh_callback */
void thresh_callback(int, void* ){
    Mat threshold_output;
    vector<cv::Point> interesting_points;
 
    /// Detect edges using Threshold
    threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY );

    for (int i = 0 ; i < threshold_output.rows ; i++) {
        for (int j = 0 ; j < threshold_output.cols ; j++) {
            if (threshold_output.at<char>(i, j) == 0) {
                interesting_points.push_back(cv::Point(i, j));
            }
        }
    }

    cout << "Lenght of arr" << endl;
    cout << interesting_points.size() << endl;

    RotatedRect minRect = minAreaRect(Mat(interesting_points));
    cout << minRect.angle << endl;
    Point2f rectPoints[4]; 
    minRect.points(rectPoints);
    for (int i = 0 ; i < 4 ; i++) {
        swap(rectPoints[i].x, rectPoints[i].y);
    }

    cvtColor(src_gray, drawing, CV_GRAY2BGR);    

    for (int i = 0 ; i < 4 ; i++) {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        line(drawing, rectPoints[i], rectPoints[(i+1) % 4], color , 1, 8);
    }

    /// Show in a window
    namedWindow("Threshold output", CV_WINDOW_AUTOSIZE);
    imshow("Threshold output", threshold_output);

    // Detected rectangle
    namedWindow("Detected rectangle", CV_WINDOW_AUTOSIZE );
    imshow("Detected rectangle", drawing );

    // on originak
    namedWindow("bla", CV_WINDOW_AUTOSIZE );
    imshow("bla", src_gray);


}