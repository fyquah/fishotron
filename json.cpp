#include <string>
#include <sstream>
#include <vector>
#include <math.h>
#include <unistd.h>
#include <chilitags/chilitags.hpp>
#include "rectangle.h"
#include "camera.h"

#ifdef OPENCV3
#include <opencv2/core/utility.hpp> // getTickCount...
#include <opencv2/imgproc/imgproc.hpp>
#endif

#include <opencv2/core/core_c.h> // CV_AA
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "FiducialMap.h"

string image_save_path;
string get_rectangle_json(double length, double width);
string get_point_json(const Point2f & p);
pair<double, double> compute_length_and_width(const vector<double> & v_length, const vector<double> & v_width);
double dst(const Point2f & a, const Point2f & b);
const int PIXEL_RATIO = 50;
int xRes = 1280;
int yRes = 720;
int thresh = 100;
// #define DEBUG 1;

string toString(int x) {
    stringstream ss;
    ss << x;
    return ss.str();
}

void callback(Mat image) {
    Mat src_gray, outputImage;
    static vector<double> v_length;
    static vector<double> v_width;
    static bool isRecording = false;
    outputImage = image.clone();
#ifdef DEBUG
    cerr << "a" << endl;
#endif

    int64 startTime = cv::getTickCount();

#ifdef DEBUG
    cerr << "B" << endl;
#endif
    // Do border detection ...

    scaleImage(image, outputImage);
    cv::cvtColor(outputImage, src_gray, CV_BGR2GRAY);
    RotatedRect minRect;
    Point2f rectPoints[4];
    Mat t_out;

#ifdef DEBUG
        cerr << "c" << endl;
#endif
    if(obtainRectangle(src_gray, thresh, minRect, t_out)) {
        minRect.points(rectPoints);
#ifdef DEBUG
        cerr << rectPoints[0] << endl;
        cerr << rectPoints[1] << endl;
        cerr << rectPoints[2] << endl;
        cerr << rectPoints[3] << endl;
#endif

        float l1 = pow(pow((rectPoints[1].x - rectPoints[0].x),2) + pow((rectPoints[1].y - rectPoints[0].y),2),0.5);
        float l2 = pow(pow((rectPoints[1].x - rectPoints[2].x),2) + pow((rectPoints[1].y - rectPoints[2].y),2),0.5);
        float len;
        if (l1 > l2) {len = l1;}
        else {len = l2;}
        len=len/50;
        cv::Scalar col = cv::Scalar(0,180,0);


        float notdeveloped = 20; //cm
        float illegal = 10;

        if (len<notdeveloped) {
            col = cv::Scalar(0,165,255);
        }

        if (len<illegal) {
            col = cv::Scalar(0,0,255);
        }
        for (int i = 0 ; i < 4 ; i++) {
            line(outputImage, rectPoints[i], rectPoints[(i+1) % 4], col , 2, 8);
        }
        cv::putText(outputImage, cv::format("%.01f", len), Point2f(50,50),
                    cv::FONT_HERSHEY_SIMPLEX, 2.0f, Scalar(0, 0, 0));

        if (isRecording) {
            circle(outputImage, Point2f(1000, 70), 35, Scalar(0, 0, 255), 25);

            double a = dst(rectPoints[0], rectPoints[1]);
            double b = dst(rectPoints[1], rectPoints[2]);
            v_length.push_back(max(a, b));
            v_width.push_back(min(a, b));
        }

    }  else {
#ifdef DEBUG
        cerr << "Unable to obtained best rectangle!" << endl;
#endif
    }

    // Finally...
    cv::imshow("DisplayChilitags", outputImage);

    // handle break conditions
    if (isRecording && v_length.size() >= 20) {
        // do forking and break
        cout << "Finish recording!" << endl;
        pid_t pid = fork();
        if (pid == 0) {
            // children process
            pair<double, double> p = compute_length_and_width(v_length, v_width);
            image_save_path =  toString(rand() % 1000) + image_save_path;
            imwrite(image_save_path.c_str(), image);
            execl("/usr/bin/python", "python", "push.py", image_save_path.c_str(), toString(p.first).c_str(), toString(p.second).c_str(), (char*) 0);
            exit(0);

        } else if(pid >= 1) {
            isRecording = false;
            v_length.clear();
            v_width.clear();
        } else if (pid == -1) {
            // error occued
            // an error occured!
        }

    } else if(!isRecording && waitKey(1) == 13) {
        cout << "Start recording!" << endl;
        isRecording = true;
        v_length.clear();
        v_width.clear();
    }
}

int main(int argc, char* argv[]){
    assert(argc >= 2);
    // Simple parsing of the parameters related to the image acquisition

    camera::setIpAddress("192.168.2.6:8080");
    image_save_path = argv[1];
    RNG rng(12345);
    int thresh = 100;
    bool isRecording = false;
    // The source of input images

    cv::namedWindow("Display");
    camera::poolCamera(callback);
    cv::destroyWindow("DisplayChilitags");
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
