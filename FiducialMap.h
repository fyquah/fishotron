#include <iostream>
#include <tuple>
#include <opencv2/imgproc/imgproc.hpp>

namespace fish {

    std::tuple<bool, float, cv::Mat> scaleImage(const cv::Mat &, cv::Mat &, cv::Size);
    cv::Point2f getPos(int, int);

}
