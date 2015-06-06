#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <queue>
#include <map>
#include <utility>
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


  // imshow(source_window, src_gray);
  createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback );
  thresh_callback( 0, 0 );
  cout << "Done" << endl;
  waitKey(0);
  return(0);
}

bool isValidPoint(Point p, const Mat & threshold_output) {
    return p.x >= 0 && p.y >= 0 && p.x < threshold_output.cols && p.y < threshold_output.rows;
}

void transverse(Point p, const Mat & threshold_output, vector<Point> & interesting_points) {
    // p is the starting point
    // this function BFS from the point closest to the centroid
    const int WINDOW_SIZE = 2;
    static map<pair<int, int>, bool> visited;

    queue<Point> q;
    q.push(p);

    while (!q.empty()) {

        Point p = q.front();
        q.pop();

        if(visited[make_pair(p.x, p.y)]) {
            continue;
        }

        visited[make_pair(p.x, p.y)] = true;

        for (int i = -WINDOW_SIZE/2 ; i <= WINDOW_SIZE/2 ; i++) {
            for (int j = -WINDOW_SIZE/2; j <= WINDOW_SIZE/2 ; j++) {
                int x = p.x + i;
                int y = p.y + j;

                if (isValidPoint(Point(x, y), threshold_output) && 
                    !visited[make_pair(x, y)] && 
                    threshold_output.at<char>(Point(x, y)) == 0) {

                    q.push(Point(x, y));
                    interesting_points.push_back(Point(x, y));
                }
            }
        }
    }
}

inline int distSq(const Point a, const Point b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

void obtainInterestingPoints(const vector<Point> & candidate_points , const Mat & threshold_output, vector<Point> & interesting_points) {

    // compute the "centroid"
    int s_x = 0;
    int s_y = 0;
    for (int i = 0 ; i < candidate_points.size() ; i++) {
        s_x += candidate_points[i].x;
        s_y += candidate_points[i].y;
    }
    long long varience = 0;
    s_x /= candidate_points.size();
    s_y /= candidate_points.size();
    Point centroid = Point(s_x, s_y);

    for (int i = 0 ; i < candidate_points.size() ; i++) {
        varience += distSq(candidate_points[i], centroid);
    }
    varience /= candidate_points.size();


    for (int i = 0 ; i < candidate_points.size() ; i++) {
        if (distSq(centroid, candidate_points[i]) < varience * 4) {
            transverse(candidate_points[i], threshold_output, interesting_points);
        }
    }    
}

void thresh_callback(int, void* ){
    Mat threshold_output;
    vector<cv::Point> interesting_points, candidate_points;
 
    /// Detect edges using Threshold
    threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY );

    for (int i = 0 ; i < threshold_output.rows ; i++) {
        for (int j = 0 ; j < threshold_output.cols ; j++) {
            if (threshold_output.at<char>(Point(j, i)) == 0) {
                candidate_points.push_back(cv::Point(j, i));
            }
        }
    }

    obtainInterestingPoints(candidate_points, threshold_output, interesting_points);

    cout << "Lenght of arr" << endl;
    cout << interesting_points.size() << endl;

    RotatedRect minRect = minAreaRect(Mat(interesting_points));
    cout << minRect.angle << endl;
    Point2f rectPoints[4]; 
    minRect.points(rectPoints);


    cvtColor(src_gray, drawing, CV_GRAY2BGR);    

    for (int i = 0 ; i < 4 ; i++) {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        line(drawing, rectPoints[i], rectPoints[(i+1) % 4], color , 1, 8);
    }

    // Detected rectangle
    namedWindow("Detected rectangle", CV_WINDOW_AUTOSIZE );
    imshow("Detected rectangle", drawing );

    namedWindow("bla", CV_WINDOW_AUTOSIZE);
    imshow("bla", threshold_output);

}