#include "rectangle.h"
#include <chilitags/chilitags.hpp>

#ifdef OPENCV3
#include <opencv2/core/utility.hpp> // getTickCount...
#include <opencv2/imgproc/imgproc.hpp>
#endif

#include <opencv2/core/core_c.h> // CV_AA
#include <opencv2/core/core.hpp>

// OpenCV goodness for I/O
#include <opencv2/highgui/highgui.hpp>

#include <iostream>


int main(int argc, char* argv[])
{
    // Simple parsing of the parameters related to the image acquisition
    int xRes = 640;
    int yRes = 480;
    int cameraIndex = 0;
    if (argc > 2) {
        xRes = std::atoi(argv[1]);
        yRes = std::atoi(argv[2]);
    }
    if (argc > 3) {
        cameraIndex = std::atoi(argv[3]);
    }
    RNG rng(12345);
    int thresh = 100;
    // The source of input images

    cv::VideoCapture capture;
    capture.open("http://c-cam.uchicago.edu/mjpg/video.mjpg");
    if (!capture.isOpened())
    {
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
    for (; 'q' != (char) cv::waitKey(1); ) {
        cout << "a" << endl;
        // Capture a new image.
        capture.read(inputImage);
        cv::Mat outputImage = inputImage.clone();

//         // Start measuring the time needed for the detection
//         int64 startTime = cv::getTickCount();

//         // Detect tags on the current image (and time the detection);
//         // The resulting map associates tag ids (between 0 and 1023)
//         // to four 2D points corresponding to the corners positions
//         // in the picture.
//         chilitags::TagCornerMap tags = chilitags.find(inputImage);

//         // Measure the processing time needed for the detection
//         int64 endTime = cv::getTickCount();
//         float processingTime = 1000.0f*((float) endTime - startTime)/cv::getTickFrequency();


//         // Now we start using the result of the detection.

//         // First, we set up some constants related to the information overlaid
//         // on the captured image
//         const static cv::Scalar COLOR(255, 0, 255);
//         // OpenCv can draw with sub-pixel precision with fixed point coordinates
//         static const int SHIFT = 16;
//         static const float PRECISION = 1<<SHIFT;

//         // We dont want to draw directly on the input image, so we clone it
        

//         for (const std::pair<int, chilitags::Quad> & tag : tags) {

//             int id = tag.first;
//             // We wrap the corner matrix into a datastructure that allows an
//             // easy access to the coordinates
//             const cv::Mat_<cv::Point2f> corners(tag.second);

//             // We start by drawing the borders of the tag
//             for (size_t i = 0; i < 4; ++i) {
//                 cv::line(
//                     outputImage,
//                     PRECISION*corners(i),
//                     PRECISION*corners((i+1)%4),
// #ifdef OPENCV3
//                     COLOR, 1, cv::LINE_AA, SHIFT);
// #else
//                     COLOR, 1, CV_AA, SHIFT);
// #endif
//             }

//             // Other points can be computed from the four corners of the Quad.
//             // Chilitags are oriented. It means that the points 0,1,2,3 of
//             // the Quad coordinates are consistently the top-left, top-right,
//             // bottom-right and bottom-left corners.
//             // (i.e. clockwise, starting from top-left)
//             // Using this, we can compute (an approximation of) the center of
//             // tag.
//             cv::Point2f center = 0.5f*(corners(0) + corners(2));
//             cv::putText(outputImage, cv::format("%d", id), center,
//                         cv::FONT_HERSHEY_SIMPLEX, 0.5f, COLOR);
//         }

//         // Some stats on the current frame (resolution and processing time)
//         cv::putText(outputImage,
//                     cv::format("%dx%d %4.0f ms (press q to quit)",
//                                outputImage.cols, outputImage.rows,
//                                processingTime),
//                     cv::Point(32,32),
//                     cv::FONT_HERSHEY_SIMPLEX, 0.5f, COLOR);

        // Do border detection ...
        cv::cvtColor(inputImage, src_gray, CV_BGR2GRAY);
        RotatedRect minRect;
        Point2f rectPoints[4]; 
        cout << "b" << endl;
        Mat t_out;

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
        }  else {
            cout << "Unable to obtained best rectangle!" << endl;
        }

        // Finally...
        cv::imshow("DisplayChilitags", outputImage);
    }

    cv::destroyWindow("DisplayChilitags");
    capture.release();

    return 0;
}