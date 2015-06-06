#include "rectangle.h"

inline int distSq(const Point a, const Point b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
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

void obtainInterestingPoints(const vector<Point> & candidate_points , const Mat & threshold_output, vector<Point> & interesting_points) {

    if(!candidate_points.size()) {
        return;
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
    Point centroid = Point(s_x, s_y);

    for (int i = 0 ; i < candidate_points.size() ; i++) {
        varience += distSq(candidate_points[i], centroid);
    }

    if(candidate_points.size()) {
        varience /= candidate_points.size();
    }

    for (int i = 0 ; i < candidate_points.size() ; i++) {
        if (distSq(centroid, candidate_points[i]) < varience * 4) {
            interesting_points.push_back(candidate_points[i]);
            // transverse(candidate_points[i], threshold_output, interesting_points);
        }
    }    
}

bool obtainRectangle(const Mat & src_gray, int thresh, RotatedRect & minRect, Mat & threshold_output) {
    // Mat threshold_output;
    vector<cv::Point> interesting_points, candidate_points;
    /// Detect edges using Threshold
    threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);
#ifdef DEBUG
    cerr << "d" << endl;
#endif
    for (int i = 0 ; i < threshold_output.rows ; i++) {
        for (int j = 0 ; j < threshold_output.cols ; j++) {
            if (threshold_output.at<char>(Point(j, i)) == 0) {
                candidate_points.push_back(cv::Point(j, i));
            }
        }
    }
#ifdef DEBUG
    cerr << "e" << endl;
#endif
    obtainInterestingPoints(candidate_points, threshold_output, interesting_points);

    if (interesting_points.size()) {
        minRect = minAreaRect(interesting_points);
    }
#ifdef DEBUG
    cerr << candidate_points.size() << endl;
#endif
    return candidate_points.size() > MINIMUM_EDGE_COUNT;
}