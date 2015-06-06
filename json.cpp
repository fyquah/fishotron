#include <string>
#include <sstream>
#include <math.h>
#include "rectangle.h"

string get_rectangle_json(const RotatedRect & minRect);
string get_point_json(const Point2f & p);
double dst(const Point2f & a, const Point2f & b);

int main(int argc, char * argv[]) {
    Mat src, src_gray;
    RotatedRect minRect;

    src = imread( argv[1], 1 );
    cv::resize(src, src, cv::Size(700, 500));
    cvtColor(src, src_gray, CV_BGR2GRAY);
    
    // output the json format;
    if(obtainRectangle(src_gray, 100, minRect)) {
        cout << get_rectangle_json(minRect) << endl;
    } else {
        cout << "{\"error\": \"failed to get best rect\"}" << endl;
    }
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

string get_rectangle_json(const RotatedRect & minRect) {
    double a, b, length, width;
    stringstream ss;
    Point2f pts[4];
    minRect.points(pts);

    a = dst(pts[0], pts[1]);
    b = dst(pts[1], pts[2]);
    length = max(a, b);
    width = min(a, b);


    ss << "{";

    ss << "\"points\":[";

    for(int i = 0 ; i < 4; i++) {
        ss << get_point_json(pts[i]);
        if (i != 3) ss << ",";
    }

    ss << "],";

    ss << "\"length\": " << length << ",";
    ss << "\"width\": " << width;

    ss << "}";
    return ss.str();
}