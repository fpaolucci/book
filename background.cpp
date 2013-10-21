#include "background.h"

int Background::isCaptured(){
        if( this->background_src==NULL ) return 0;
        else return 1;
}
IplImage* Background::getBackgroundSource (){
        return this->background_src;	
}
IplImage* Background::getBackground () {
        return this->background;	
}
int Background::captureBackground( IplImage* src) {
        if( src == NULL) 
                return 0;
        if (background_src!=0 ) {
                cvReleaseImage(&background_src);
                cvReleaseImage(&background);
        } 

        //image data -> MEMORIZZO LE CARATTERISTICHE DELL'IMMAGINE
        size= cvGetSize( src);
        size_half= cvSize( size.width/2.0, size.height/2.0);
        depth=src->depth;
        nChannels=src->nChannels;

        //background_src -> MEMORIZZO IL BACKGROUND ORIGINALE
        background_src= cvCreateImage( size, depth, nChannels);
        cvSetZero( this->background_src);
        cvCopy(src, this->background_src);
        //cvCvtColor( background_src, background_src, CV_BGR2HSV);
        cvSmooth ( this->background_src, this->background_src, CV_GAUSSIAN, 7,7);

        //background -> PROCESSO IL BACKGROUND PER EVITARE ERRORI E RUMORI
        background= cvCreateImage( size_half, depth, nChannels); 
        cvPyrDown( background_src, background, 7);
        cvAddS(background,cvScalar(30,30,30), background);

        return 1;
}
int Background::deleteBackground( IplImage* src, IplImage* dst){
        if ( src->width!=size.width && src->height!= size.height && src->depth!= depth) 
                return 0;
        //cvCvtColor( src, src, CV_BGR2HSV);
        cvSmooth(src, src, CV_GAUSSIAN, 7,7 );
        IplImage* pyr=cvCreateImage( size_half, depth, nChannels);
        cvSetZero( pyr);
        cvPyrDown( src, pyr, 7);
        cvAddS(pyr,cvScalar(30,30,30), pyr);

        cvAbsDiff( pyr, background, pyr);

        //cvSmooth( pyr, pyr, CV_MEDIAN, 7,7);
        IplConvKernel* dilationElement = cvCreateStructuringElementEx( 5,5,3,3, CV_SHAPE_RECT , 0 );
        IplConvKernel* erosionElement = cvCreateStructuringElementEx( 5,5,3,3, CV_SHAPE_RECT , 0 );
        // Dilation adds a layer on, and returns things to the correct size.
        cvDilate(pyr,pyr,dilationElement,1);
        // Erosion peels a layer of pixels off, and makes small regions disappear
        //cvErode(pyr,pyr,erosionElement,2);

        cvCvtColor(pyr, dst, CV_RGB2GRAY);
        cvThreshold( dst, dst, 30 ,250, CV_THRESH_BINARY); 
        //cvAdaptiveThreshold( dst, dst, 30,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY_INV,3,5);

        //cvCopyImage( pyr, dst);
        return 1;
}
int Background::deleteBaseObjects(  IplImage* dst) {
        double cut= 10.0;
        CvRect r;
        r.width=dst->width;
        r.height=(int)(dst->height/cut);
        r.x=0;
        r.y=dst->height-r.height;
        cvSetImageROI( dst, r);
        cvZero(dst);
        cvResetImageROI( dst);
        return 1;

}
