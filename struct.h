#ifndef STRUCT_H
#define STRUCT_H

#include <cv.h>
#include <cxcore.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

typedef enum Hand_Mov {
HAND_NOP= 0,
HAND_TO_SX= -1,
HAND_TO_DX= 1
} Hand_Mov ;

int mod (int val, int m);
IplImage* imgPyrDown(IplImage* img);
IplImage* imgPyrUp(IplImage* img);
#endif