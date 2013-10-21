#include "stdafx.h"
#include "background.h"
#include "tracking.h"
#include "book.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int  _tmain(int argc, char** argv) {
	//INIZIALIZZO I NOMI DEI FILES
	char* files[]={"./book/gioconda.jpg","./book/rembrandt.jpg","./book/renoir.jpg","./book/dali.jpg","./book/vangogh.jpg","./book/manet.jpg","./book/davinci.jpg","./book/urlodimunch.jpg"};
	int n=8;


	IplImage* motion = 0; 
        CvCapture* capture = 0;
        char* result_capture;
	IplImage* image=NULL;
	int action=-1;
	int lastaction=-1;
	CvRect* handROI=0;
	int actioncont=0;
	char* filename="template.jpg";
	IplImage* fist_template=0;
	IplImage* fist_mask=0;

        if( argc == 1 ) {
                capture = cvCaptureFromCAM( 0 );
		result_capture= "Capture from camera status: ";
	}
        else if( argc == 2 ) {
		capture = cvCaptureFromAVI( argv[1] );
		result_capture= "Capture from video status: ";
	} 

	if( capture ) {
		printf("%s ok\n", result_capture);
		printf( "<<<\t\tWELCOME GESTURE INTERFACES MENU\t\t>>>\n\t1- Start Background Calibration - Press button c\n\t2- Start Hand Tracking - Press button s\n");
                cvNamedWindow( "Background", 1 );
		cvMoveWindow("Background", 760, 60);
		cvNamedWindow( "Process", 1);
		cvMoveWindow( "Process", 100,360);
		cvNamedWindow("Debug1",1);
		cvMoveWindow( "Debug1", 430, 360);
		cvNamedWindow("Debug2",1);
		cvMoveWindow( "Debug2", 760, 360);

		Background* bg=new Background();
		Tracking* tracking=new Tracking();
		Book* book= new Book();
		book->init(files,n);
                for( ; image=cvRetrieveFrame(capture) ; ) {
                    action= cvWaitKey(10);
                    if( action==27 ) 
                            break;
                    if (action!=-1 && action==lastaction) 
                            actioncont=(actioncont+1)%2;
                    else if( action!=-1) {
                            lastaction=action;
                            actioncont=0;
                    }
                    switch (lastaction) {
                            case ((int)'C'):
                            case ((int)'c'):
                                    if( action!=-1 && actioncont==0) {
                                            printf("c: Calibration started-> Capturing Screenshot background\n");
                                    }
                                    else if( action!=-1 && actioncont==1){
                                            for( int i=0; i<20; i++) cvGrabFrame( capture);
                                            bg->captureBackground(image);
                                            printf("c: Calibration finished-> Background captured\n");
                                    }
                                    cvShowImage( "Background", bg->getBackground() );
                                    break;
                            case ((int)'S'):
                            case ((int)'s'):
                            {
                                            //INIZIALIZZAZIONE DELLA FASE DI TEMPLATING
                                    if( !bg->isCaptured() ) {
                                    if (action!=-1) printf("s: Calibration failed -> Press c to capture background first!\n");
                                    break;
                                    }
                                    if( action!=-1 && actioncont==0) {
                                            printf("s: Calibration started -> Searching for subject hand\n");
                                            handROI = &(cvRect(0,0,0,0));
                                    }
                                            IplImage* mask=cvCreateImage( cvSize(image->width/2.0 ,image->height/2.0), image->depth, 1) ;
                                            cvSetZero( mask);
                                            bg->deleteBackground(image, mask);
                                            cvShowImage("Process", mask);

                                            IplImage* result=cvCreateImage( cvSize(image->width/2.0 ,image->height/2.0), image->depth, 3);
                                            tracking->handTemplate( image, mask, result, handROI);
                                            cvShowImage( "Debug1",result);

                                            //Salva il template e genera il file
                                    if ( action!=-1 && actioncont==1 )  {
                                            IplImage* pyr=cvCreateImage( cvSize(image->width/2.0 ,image->height/2.0), image->depth, 3);
                                            cvPyrDown( image, pyr, 7);
                                            tracking->handTemplateSave(filename, pyr , handROI);
                                            tracking->handTemplateSave("template1.jpg", mask, handROI);
                                            printf("s: Calibration finished -> Subject hand template generated in \'%s\'\n", filename);
                                            fist_template= cvLoadImage( filename);
                                            fist_mask= cvLoadImage( "template1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
                                    }
                            }
                                    break;
                            case ((int)'T'):
                            case ((int)'t'):
                            {
                                    if( (!bg->isCaptured() || fist_template==0) ) {
                                            if( action!=-1) printf("t: Tracking failed -> Calibration phase isn't completed.\nRepeat background and template phase!\n");
                                            break;
                                    }
                                    if( action!=-1 && actioncont==0) {
                                            printf("t: Tracking started -> Inizio rilevamento della mano\n");
                                    }
                                    IplImage* mask=cvCreateImage( cvSize(image->width/2.0 ,image->height/2.0), image->depth, 1) ;
                                    IplImage* pyr=cvCreateImage( cvSize(image->width/2.0 ,image->height/2.0), image->depth, 3) ;
                                    cvSetZero( mask);
                                    cvSetZero( pyr);

                                    //MODIFICO IMMAGINE
                                    cvSmooth(image, image, CV_GAUSSIAN, 7,7);
                                    cvPyrDown( image, pyr, 7);
                                    bg->deleteBackground(image, mask);
                                    bg->deleteBaseObjects(mask);
                                    cvShowImage("Debug1", mask);
                                    tracking->handStartup( mask, fist_mask);
                                    IplImage* res= tracking->handRecognition( pyr, mask, fist_template, fist_mask);
                                    Hand_Mov hmov = tracking->handCheckMovement(2.0,  pyr);
                                    if( hmov==HAND_TO_DX) printf( "DESTRA\n");
                                    if( hmov==HAND_TO_SX) printf( "SINISTRA\n");
                                    cvShowImage( "Debug2", book->readInput(hmov,clock()/1000.0));
                                    cvShowImage("Process", pyr);
                                    //cvShowImage("Debug1", res);
                                    if (action!=-1 && actioncont==1) {
                                            printf("t: Tracking stopped -> Rilevamento mano terminato.\n");
                                    }
                            }
                                    break;
                            default: 
                                    break;
                    }
                }
                cvReleaseCapture( &capture );
		cvDestroyWindow("Process");
                cvDestroyWindow( "Background" );
		cvDestroyWindow("Debug1");
                cvDestroyWindow( "Debug2" );
        }
	else printf("%s failed\n", result_capture);
	//system("PAUSE");
        return 0;
}