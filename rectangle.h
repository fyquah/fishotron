#ifndef _RECTANGLE
#define _RECTANGLE

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


namespace fish {

const unsigned MINIMUM_EDGE_COUNT = 500;
static inline int distSq(const cv::Point & a, const cv::Point & b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}
static inline float distPoints(const cv::Point & a, const cv::Point & b) {
    return std::sqrt(distSq(a, b));
}

// calculates the minimum bounding rectangle of the object
bool obtainRectangle(
    const cv::Mat &, cv::RotatedRect &, int
);
std::tuple<bool, float> calibrate(cv::Mat);

}



#endif
