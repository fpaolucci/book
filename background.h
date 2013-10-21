#ifndef BACKGROUND_H
#define BACKGROUND_H
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

class Background {
	IplImage* background_src;
	IplImage* background;
	IplImage* background_ch1;
	IplImage* background_hsv;
	CvSize size_half;
	CvSize size;
	CvScalar  hsv_min; 
	CvScalar  hsv_max ;
	int depth;
	int nChannels;

public:
	inline Background(){
			this->background_src=0;
			this->background=0;
			hsv_min = cvScalar(127, 57, 60);
			hsv_max = cvScalar(183, 154, 203);
	}
	int isCaptured();
	IplImage* getBackgroundSource ();
	IplImage* getBackground () ;
	int captureBackground( IplImage* src) ;
	int deleteBackground( IplImage* src, IplImage* dst);
	int deleteBaseObjects(  IplImage* dst) ;
};
#endif