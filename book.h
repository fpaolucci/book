#ifndef BOOK_H
#define BOOK_H
#include <cv.h>
#include <cxcore.h>
#include <struct.h>
#include <stdio.h>
#include <string.h>

class Book{
public:
        IplImage** img;
	int dim;
	char** files;
	int pos;
	int width,heigth;
	double stime;
	double animtime;
	Hand_Mov mov;

	inline Book(){
	img=0;
	files=0;
	dim=0;
	pos=0;
	stime=0;
	animtime=1.6;
	width=800;
	heigth=600;
	mov=HAND_NOP;
	}

	void init(char** filename, int num);
	IplImage* print(int p);
	IplImage* swap(int suc, double curtime);
	IplImage* readInput(Hand_Mov hmov, double curtime);
};
#endif