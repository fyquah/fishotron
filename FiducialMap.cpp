#include "FiducialMap.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "chilitags/chilitags.hpp"

#include <iostream>

using namespace std;
using namespace cv;


Point2f getPos(int fidnum,int cornum) {
	//Size fiddist = Size(1235,790); //image size 1135, 690
	//Size fidsize = Size(100,100);
	Size fiddist = Size(617,395); //image size 1135, 690
	Size fidsize = Size(50,50);
	//taking the lower right of the top left fiducial as (0,0)
	float fidx = (fidnum==1 || fidnum==2)*fiddist.width;
	float fidy = (fidnum==2 || fidnum==3)*fiddist.height;
	float cornerx = (cornum==0 || cornum == 3)*-fidsize.width;
	float cornery = (cornum==0 || cornum == 1)*-fidsize.height;
	return Point2f(fidx+cornerx,fidy+cornery);
}

int scaleImage(Mat input, Mat &output) {
	//So far, A4 sheet will be tags 8,9,10,11 [OFFSET = 2]
	int offset; //A4 = 2
	int tagCount = 0;
	Mat_<Point2f> mats[4];
	int exists[4] = {0,0,0,0};
	
    for (const auto &tag : chilitags::Chilitags().find(input)) {
		tagCount+=1;
        int id = tag.first;
		offset = id/4;
		id = (id-8)/2;
		if (id>=4) {
			continue;
		}
        Mat_<Point2f> corners(tag.second);
		mats[id] = corners;
		exists[id] = 1;
		if (tagCount>=4) {
			break;
		}
    }
	//SEGFAULT BUG BEFORE HERE
	
	if (tagCount==0) {
		output = input.clone();
		//cerr << "FUCK IT ALL" << endl;
		return -1;
	}
		
	//first quality points vs second quality points.
	int numpts = 0;
	Point2f src_p[4];
	Point2f dst_p[4];
	for (int i = 0; i<4; i++) {
		//hunt for first quality pts
		if (exists[i]) {
			
			src_p[numpts]=mats[i](2); //lower right point
			dst_p[numpts]=getPos(i,2);
			numpts+=1;
		}
	}
	int curFid = 0;
	while (numpts!=4) { //loop over each fiducial
		if (exists[curFid]) {
			for (int i = 0;i<4;i==1?i+=2:i++) {
#ifdef DEBUG
				cerr << i;
#endif
				//loop over the non-primary points [everything but 2]
				src_p[numpts]=mats[curFid](i);
				dst_p[numpts]=getPos(curFid,i);
				numpts+=1;
				if (numpts==4 || tagCount==2) {
					break;
				}
			}
		}
		curFid+=1;
	}
#ifdef DEBUG
	cerr << "Source:" << endl;
	for (int i = 0; i<4; i++) {
		cerr << i << " -- " << src_p[i].x << " -- " << src_p[i].y << endl;
	}
	cerr << "Destination:" << endl;
	for (int i = 0; i<4; i++) {
		cerr << i << " -- " << dst_p[i].x << " -- " << dst_p[i].y << endl;
	}
#endif


	Mat trans = getPerspectiveTransform(src_p,dst_p);
	int myradius=5;
	
	warpPerspective(input,output,trans,Size(567.0f,345.0f));
	
	return offset;	
	
}
