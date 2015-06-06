#include <string>
#include <sstream>
#include <vector>
#include <math.h>
#include "rectangle.h"
#include <chilitags/chilitags.hpp>

#ifdef OPENCV3
#include <opencv2/core/utility.hpp> // getTickCount...
#include <opencv2/imgproc/imgproc.hpp>
#endif

#include <opencv2/core/core_c.h> // CV_AA
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "FiducialMap.h"

string get_rectangle_json(double length, double width);
string get_point_json(const Point2f & p);
pair<double, double> compute_length_and_width(const vector<double> & v_length, const vector<double> & v_width);
double dst(const Point2f & a, const Point2f & b);
const int PIXEL_RATIO = 50;

int main(int argc, char* argv[])
{
    assert(argc >= 2);
    // Simple parsing of the parameters related to the image acquisition
    int xRes = 1280;
    int yRes = 720;
    int cameraIndex = 1;
    string image_save_path = argv[1];
    RNG rng(12345);
    int thresh = 100;
    // The source of input images
    vector<double> v_length, v_width;
    cv::VideoCapture capture;
    capture.open(cameraIndex);
    //capture.open("http://10.20.41.200:8080/video?x.mjpeg");
    if (!capture.isOpened()) {
        std::cerr << "Unable to initialise video capture." << std::endl;
        return 1;
    }
#ifdef OPENCV3
    capture.set(cv::CAP_PROP_FRAME_WIDTH, xRes);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, yRes);
#else
    capture.set(CV_CAP_PROP_FRAME_WIDTH, xRes);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, yRes);
#endif
    cv::Mat inputImage, src_gray;

    // The tag detection happens in the Chilitags class.
    chilitags::Chilitags chilitags;

    // The detection is not perfect, so if a tag is not detected during one frame,
    // the tag will shortly disappears, which results in flickering.
    // To address this, Chilitags "cheats" by keeping tags for n frames
    // at the same position. When tags disappear for more than 5 frames,
    // Chilitags actually removes it.
    // Here, we cancel this to show the raw detection results.
    chilitags.setFilter(0, 0.0f);

    cv::namedWindow("DisplayChilitags");
    // Main loop, exiting when 'q is pressed'
    for (int fc = 0; 'q' != (char) cv::waitKey(1) && v_length.size() < 15; ++fc) {
        cout << "a" << endl;
        capture.read(inputImage);
        cv::Mat outputImage;
        scaleImage(inputImage,outputImage);
        int64 startTime = cv::getTickCount();

        cout << "B" << endl;
        // Do border detection ...

        cv::cvtColor(outputImage, src_gray, CV_BGR2GRAY);
        RotatedRect minRect;
        Point2f rectPoints[4]; 
        Mat t_out;

        cout << "c" << endl;
        if(obtainRectangle(src_gray, thresh, minRect, t_out)) {
            minRect.points(rectPoints);
            cout << rectPoints[0] << endl;
            cout << rectPoints[1] << endl;
            cout << rectPoints[2] << endl;
            cout << rectPoints[3] << endl;
            
            for (int i = 0 ; i < 4 ; i++) {
                cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
                line(outputImage, rectPoints[i], rectPoints[(i+1) % 4], color , 1, 8);
            }  
            double a = dst(rectPoints[0], rectPoints[1]);
            double b = dst(rectPoints[1], rectPoints[2]);
            v_length.push_back(max(a, b));
            v_width.push_back(min(a, b));

        }  else {
            cout << "Unable to obtained best rectangle!" << endl;
        }

        // Finally...
        cv::imshow("DisplayChilitags", outputImage);
    }

    cv::destroyWindow("DisplayChilitags");
    capture.release();

    pair<double, double> p = compute_length_and_width(v_length, v_width);
    cout << get_rectangle_json(p.first, p.second) << endl;
    imwrite(image_save_path.c_str(), inputImage);

    return 0;
}

pair<double, double> compute_length_and_width(const vector<double> & v_length, const vector<double> & v_width) {
    double sum_length, sum_width, var_length, var_width, length, width;
    var_width = var_length = length = width = sum_length = sum_width = 0;

    for (int i = 0 ; i < v_length.size() ; i++) {
        sum_length += v_length[i];
        sum_width += v_width[i];
    }
    double mean_length = sum_length / v_length.size();
    double mean_width = sum_width / v_width.size();

    for (int i = 0 ; i < v_length.size() ; i++) {
        var_length += (v_length[i] - mean_length) * (v_length[i] - mean_length);
        var_width += (v_width[i] - mean_width) * (v_width[i] - mean_width);
    }

    sum_length = 0;
    sum_width = 0;
    int count_length = 0;
    int count_width = 0 ;
    // remove the outlier

    for (int i = 0 ; i < v_length.size() ; i++) {
        if((v_length[i] - mean_length) * (v_length[i] - mean_length) < 4 * var_length) {
            sum_length += v_length[i];
            count_length++;
        }
        if((v_width[i] - mean_width) * (v_width[i] - mean_width) < 4 * var_width) {
            sum_width += v_width[i];
            count_width++;
        }
    }

    if (count_length != 0) {
        length = sum_length / count_length;
    } else {
        length = mean_length;
    }

    if (count_width != 0) {
        width = sum_width / count_width;
    } else {
        width = mean_width;
    }

    // compute the right average

    return make_pair(length / PIXEL_RATIO, width / PIXEL_RATIO);
}


double dst(const Point2f & a, const Point2f & b) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    return sqrt(x * x + y * y);
}

string get_point_json(const Point2f & p) {
    stringstream ss;
    ss << "{";
    ss << "\"x\": " << p.x << ",";
    ss << "\"y\": " << p.y;
    ss << "}";
    return ss.str();
}

string get_rectangle_json(double length, double width) {
    stringstream ss;

    ss << "{";

    // ss << "\"points\":[";

    // for(int i = 0 ; i < 4; i++) {
    //     ss << get_point_json(pts[i]);
    //     if (i != 3) ss << ",";
    // }

    // ss << "],";

    ss << "\"length\": " << length << ",";
    ss << "\"width\": " << width;

    ss << "}";
    return ss.str();
}
