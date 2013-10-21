#ifndef TRACKING_H
#define TRACKING_H

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "struct.h"
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <sstream>
#include <time.h>

#define MAX_AREA 50000
#define MIN_AREA 10000
 
class Tracking {
	//VARIABILI DI MOVIMENTO
	int dim;
	CvRect* buf_rect;
	double* time_rect;
	int last;
	double init;
	int frames;
	FILE* f;

	//RECOGNITION VARIABLES
	IplImage* cannymask;
	IplImage* cannyhand_mask;
	IplImage* mask;
	IplImage* hand_mask;
	CvSize size;
	CvSize hand_size;
	int depth;
	int nChannels;
	int hperimeter;
	int harea;


public:
	Tracking();
	Hand_Mov handCheckMovement( double interval, IplImage* dst );
	IplImage* handStartup ( IplImage* mask, IplImage* hand_mask) ;
	int handCheckUp() ;
	IplImage*  handRecognition(IplImage* src,IplImage* mask, IplImage* hand_templ, IplImage* hand_mask);
	CvRect contornArea( IplImage* add_rect, CvMemStorage* stor, CvSeq* cont);
	int handTemplateSave( char* file, IplImage* src, CvRect* hand);
	int handTemplate( IplImage* src, IplImage* mask, IplImage* dst, CvRect* handROI);
	int contornAreaMax( IplImage* add_rect, CvMemStorage* stor, CvSeq* cont, double ratio, CvRect* rect);
};
#endif