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

#include "FiducialMap.h"
#include <cmath>

int main(int argc, char* argv[])
{
    // Simple parsing of the parameters related to the image acquisition
    int xRes = 1280;
    int yRes = 720;
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
    capture.open(0);
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
    //chilitags.setFilter(0, 0.0f);

    cv::namedWindow("DisplayChilitags");
    // Main loop, exiting when 'q is pressed'
    for (int fc = 0; 'q' != (char) cv::waitKey(1); ++fc) {
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
                        cv::FONT_HERSHEY_SIMPLEX, 2.0f, col);
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
