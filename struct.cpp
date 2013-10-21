#include "struct.h"

int mod (int val, int m){
		int q= (val/m)-1;
		if( val<0) return val-(q*m);
		else return val%m;
}
IplImage* imgPyrDown(IplImage* img){
	assert(img->width%2==0 && img->height%2==0);
	IplImage* out= cvCreateImage(cvSize(img->width/2,img->height/2),img->depth,img->nChannels);
	cvPyrDown(img,out, IPL_GAUSSIAN_5x5);
	return out;
}
IplImage* imgPyrUp(IplImage* img){
	assert(img->width%2==0 && img->height%2==0);
	IplImage* out= cvCreateImage(cvSize(img->width*2,img->height*2),img->depth,img->nChannels);
	cvPyrUp(img,out, IPL_GAUSSIAN_5x5);
	return out;
}