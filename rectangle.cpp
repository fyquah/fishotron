#include "Rectangle.h"

namespace fish {

// runs a statistical test on candidate_points to determine which are the "interesting points"
static std::vector<cv::Point> obtainInterestingPoints(
        const std::vector<cv::Point> & candidate_points ,
        const cv::Mat & threshold_output
) {
    std::vector<cv::Point> interesting_points;

    if(!candidate_points.size()) {
        return interesting_points;
        // premature return on empty array input
    }

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
    cv::Point centroid = cv::Point(s_x, s_y);

    for (int i = 0 ; i < candidate_points.size() ; i++) {
        varience += distSq(candidate_points[i], centroid);
    }

    if(candidate_points.size()) {
        varience /= candidate_points.size();
    }

    for (int i = 0 ; i < candidate_points.size() ; i++) {
        if (distSq(centroid, candidate_points[i]) < varience * 4) {
            interesting_points.push_back(candidate_points[i]);
        }
    }

    return interesting_points;
}


bool obtainRectangle(const cv::Mat & src_gray, cv::RotatedRect & minRect, int thresh) {
    cv::Mat threshold_output;
    // Mat threshold_output;
    std::vector<cv::Point> interesting_points, candidate_points;

    /// Detect edges using Threshold

    cv::Canny(src_gray, threshold_output, thresh, thresh*2, 3);
    // threshold(src_gray, threshold_output, thresh, 255, cv::THRESH_BINARY);

    // THIS IS A BOTTLE NECK, HOW TO DO THIS IN PARALLEL?
    for (int i = 0 ; i < threshold_output.rows ; i++) {
        for (int j = 0 ; j < threshold_output.cols ; j++) {
            if (threshold_output.at<char>(cv::Point(j, i)) >= 0) {
                candidate_points.push_back(cv::Point(j, i));
            }
        }
    }

    interesting_points = obtainInterestingPoints(candidate_points, threshold_output);

    if (interesting_points.size()) {
        minRect = minAreaRect(interesting_points);
    }

    return candidate_points.size() > MINIMUM_EDGE_COUNT;
}

// not working yet :(
class ParallelPushIfNullBody: public cv::ParallelLoopBody {
private:
    std::vector<cv::Point> & v;
    cv::Mat & m;
public:
    ParallelPushIfNullBody(std::vector<cv::Point> & v, cv::Mat & m) : v(v), m(m) {}

    virtual void operator()(const cv::Range & r) const {
        for (int i = r.start ; i < r.end ; ++i) {
            for (int j = 0 ; j < m.cols ; ++j) {
                if (m.at<char>(cv::Point(j, i)) == 0) {
                    v.push_back(cv::Point(j, i));
                }
            }
        }
    }
};

}

