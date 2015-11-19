#include "Rectangle.h"
#include "FiducialMap.h"

namespace fish {

std::tuple<bool, float> calibrate(cv::Mat m) {
    float sum = 0.0;
    bool isScaled;
    float ppmm;
    cv::Mat scaled, output, transformation;

    float lo = 0;
    float hi = 200;
    std::tie(isScaled, ppmm, transformation) = scaleImage(m, scaled, cv::Size(640, 480));
    if (!isScaled) return std::make_tuple(false, 100);

    // binary search for the best possible threshold value
    while (hi - lo >= 0.5) {
        float mid = (lo + hi) / 2;
        int number_of_edges = 0;
        cv::Canny(scaled, output, int(mid), 200, 3);


        for (int i = 0 ; i < output.rows ; i++) {
            if (i >= 0.1 * float(output.rows) && i <= 0.9 * float(output.rows)) {
                for (int j = 0 ; j < output.cols ; j++) {
                    if (j >= 0.1 * float(output.cols) &&
                            j <= 0.9 * float(output.cols) &&
                            output.at<uint8_t>(cv::Point(j, i)) > 0) {
                        number_of_edges++;
                    }
                }

            }
        }
        // evaluate the quality of the filter (By counting the number of edges)


        std::cout << number_of_edges << std::endl;
        if (number_of_edges < 10) {
            hi = mid;
        } else {
            lo = mid;
        }
    }

    // any other possible callibrations?

    return std::make_tuple(true, lo);
}

// runs a statistical test on candidate_points to determine which are the "interesting points"
static std::vector<cv::Point> obtainSignificantEdges(
    const cv::Mat & edgeDetectionOutput,
    const cv::Mat & originalImage
) {
    std::vector<cv::Point> candidate_points, interesting_points;

    for (int i = 0 ; i < edgeDetectionOutput.rows ; i++) {
        if (i >= 0.1 * float(edgeDetectionOutput.rows) && i <= 0.9 * float(edgeDetectionOutput.rows)) {
            for (int j = 0 ; j < edgeDetectionOutput.cols ; j++) {
                if (j >= 0.1 * float(edgeDetectionOutput.cols) &&
                        j <= 0.9 * float(edgeDetectionOutput.cols) &&
                        edgeDetectionOutput.at<uint8_t>(cv::Point(j, i)) > 0) {
                    candidate_points.push_back(cv::Point(j, i));
                }
            }

        }
    }

    std::cout << candidate_points.size() << std::endl;

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
    cv::Mat threshold_output, blurred;
    std::vector<cv::Point> interesting_points;

    /// Detect edges using Threshold

    cv::blur(src_gray, blurred, cv::Size(3, 3));
    cv::Canny(blurred, threshold_output, 60, 200, 3);
    // threshold(src_gray, threshold_output, thresh, 255, cv::THRESH_BINARY);
    imshow("Threshold output", threshold_output);

    interesting_points = obtainSignificantEdges(threshold_output, blurred);

    if (interesting_points.size()) {
        minRect = minAreaRect(interesting_points);
    }

    return interesting_points.size() > MINIMUM_EDGE_COUNT;
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

