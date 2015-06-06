#ifndef _RECTANGLE
#define _RECTANGLE

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

const unsigned MINIMUM_EDGE_COUNT = 1000;
inline int distSq(const Point a, const Point b);

bool isValidPoint(Point p, const Mat & threshold_output);
void transverse(Point p, const Mat & threshold_output, vector<Point> & interesting_points);
void obtainInterestingPoints(const vector<Point> & candidate_points , const Mat & threshold_output, vector<Point> & interesting_points);
bool obtainRectangle(const Mat & src_gray, int thresh, RotatedRect & minRect);

#endif