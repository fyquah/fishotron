#include "FiducialMap.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "chilitags/chilitags.hpp"
#include "PageSizes.h"

namespace fish {

static chilitags::Chilitags chiliT;

cv::Point2f getPos(int fidnum, int cornum, cv::Size outputSize, int fidWidth) {
    //int fidnum goes from 0-7
    //So... outputSize defines the position of the draw size, and fidWidth defines in PIXELS the width of the fiducial

    cv::Size fidsize = cv::Size(fidWidth,fidWidth);
    //taking the lower right of the top left fiducial as (0,0)
    outputSize.width += fidWidth;
    outputSize.height +=fidWidth; //I think this is right. Hm.
    float fidx, fidy;
    if (fidnum == 0 || fidnum>=6) fidx = 0;
    else if (fidnum == 1 || fidnum == 5) fidx = (outputSize.width / 2);
    else if (fidnum>=2 && fidnum <=4) fidx = outputSize.width;
    else {
        //error!
        std::cout << "Value of fidnum exceeded 7" << std::endl;
    }

    if (fidnum <=2) fidy = 0;
    else if (fidnum == 3 || fidnum == 7) fidy = (outputSize.height / 2);
    else if (fidnum >=4 && fidnum<=6) fidy = outputSize.height;
    else {
        std::cout << "Value of fidnum exceeded 7" << std::endl;
    }

    float cornerx = (cornum==0 || cornum == 3)*-fidsize.width;
    float cornery = (cornum==0 || cornum == 1)*-fidsize.height;
    return cv::Point2f(fidx+cornerx,fidy+cornery);
}

std::tuple<bool, float, cv::Mat> scaleImage(const cv::Mat &input, cv::Mat &output, cv::Size outputSize) {
    bool rotateOutput = false;

    if (outputSize.height > outputSize.width) {
        std::swap(outputSize.height, outputSize.width);
        rotateOutput = true;
    }

    //So far, A4 sheet will be tags 8,9,10,11 [OFFSET = 2] [Well, they should be. they aren't. see below]

    //We have three sizes. The input image size, which is almost irrelevant.
    //The output image size, as passed in.
    //The largest possible image we can fit into the output, which has the *same* width *or* height as the output image size. This is the 'draw size'

    //We draw to a mat of size output - then draw two rectangles over it.

    int offset = -1; //A4 = 1
    int tagCount = 0;
    cv::Mat_<cv::Point2f> mats[8];
    int exists[8] = {0,0,0,0,0,0,0,0};
    //chilitags.setFilter(30, 0.5f);
    for (const auto &tag : chiliT.find(input)) { //Collect the chillitags
        tagCount+=1;
        int id = tag.first;
        offset = id/8; //offset 1;
        id = id%8; //Tags are 8..15
        cv::Mat_<cv::Point2f> corners(tag.second);
        mats[id] = corners;
        exists[id] = 1;
        if (tagCount>=8) {
            break;
        }
    }

    if (tagCount<4) {
        resize(input, output, outputSize);
        return std::make_tuple(false, -1, cv::Mat());
    }

    //Now we have the tag data, calculate the size of the draw size
    int widthmm = pageSizes[offset][PageWidth];
    int heightmm = pageSizes[offset][PageHeight];
    int fiducialdim = pageSizes[offset][FiducialWidth];

    float paperRatio, outputRatio;
    paperRatio = float(widthmm) / float(heightmm);
    outputRatio = float(outputSize.width) / float(outputSize.height);
    cv::Size drawSize;
    bool letterbox = false; //if this is false, it means bars on right and left. If true, bars on top and bottom.

    if (paperRatio<(outputRatio*0.98)) {
        //this means that there will be rectangles on the left and right.
        drawSize.height = outputSize.height;
        drawSize.width = paperRatio * outputSize.height;
        letterbox = false;
    } else if (paperRatio>(outputRatio*1.02)) {
        //this implies rectangles on top and bottom
        drawSize.width = outputSize.width;
        drawSize.height = (1.0/paperRatio)*outputSize.width;
        letterbox = true;
    } else {
        //the outputsize is close enough in ratio that we can directly map it. W00t.
        drawSize.width = outputSize.width;
        drawSize.height = outputSize.height;
        letterbox = false; //we could set it as either
    }
    float ppmm = float(drawSize.width)/float(widthmm); //pixels per mm in real life
    int fidwidthpixels = int(ppmm*fiducialdim);

    //first quality points vs second quality points.
    int numpts = 0;
    cv::Point2f src_p[4];
    cv::Point2f dst_p[4];
    int preferenceOrder[8] = {0,2,4,6,1,3,5,7};
    for (int i = 0; i<8; i++) {
        //hunt for first quality pts
        int hunted = preferenceOrder[i];
        if (exists[hunted]) {
            src_p[numpts]=mats[hunted](numpts); //lower right point
            dst_p[numpts]=getPos(hunted,numpts,drawSize,fidwidthpixels);
            numpts+=1;
        }
        if (numpts>=4) {
            break;
        }
    }

    // We're now calculating how to translate the points,
    // if the output is a different ratio to the input.
    int translationY = letterbox ? ((outputSize.height-drawSize.height)/2) : 0;
    int translationX = letterbox ? 0 : ((outputSize.width-drawSize.width)/2);

    for (int i = 0; i<4; i++) {
        //Now we need to translate the points as expected
        cv::Point2f p = dst_p[i];
        p.x += translationX;
        p.y += translationY;
        dst_p[i]=p;
    }

    cv::Mat transform = getPerspectiveTransform(src_p,dst_p);

    output = cv::Mat(outputSize,input.type());
    cv::warpPerspective(
        input, output, transform, outputSize,
        cv::INTER_LINEAR,cv::BORDER_CONSTANT,
        cv::Scalar(255,255,255)
    );

    //Finally, draw some rectangles!
    if (letterbox) {
        cv::rectangle(output, cv::Point(0,0), cv::Point(outputSize.width,translationY), cv::Scalar(255, 255, 255), CV_FILLED); //Blue
        cv::rectangle(output, cv::Point(0,translationY+drawSize.height), cv::Point(outputSize.width,outputSize.height), cv::Scalar(255,255,255), CV_FILLED); //Red
    } else {
        cv::rectangle(output, cv::Point(0,0), cv::Point(translationX,outputSize.height), cv::Scalar(255,255,255),CV_FILLED); //Black
        cv::rectangle(output, cv::Point(translationX+drawSize.width,0), cv::Point(outputSize.width, outputSize.height), cv::Scalar(255,255,255), CV_FILLED); //Green
    }

    // Then rotate it if the original plan was to rotate it to fit the output dimensions:
    if (rotateOutput) {
        cv::Mat res = output.t();
        cv::flip(res, res, 1);
        res.copyTo(output);
    }

    return std::make_tuple(true, ppmm, getPerspectiveTransform(dst_p, src_p)
    );
}

}

