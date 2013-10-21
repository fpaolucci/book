#include "book.h"

void Book::init(char** filename, int num){
        dim=num;
        files=(char**) malloc(num*sizeof(char*));
        memset(files,0, num*sizeof(IplImage*) );
        memcpy(files, filename, dim*sizeof(char*));
        img=(IplImage**) malloc(num*sizeof(IplImage*));
        memset(img,0, num*sizeof(IplImage*) );
        for(int i=0; i<dim; i++){
                img[i]=cvLoadImage(files[i]);
        }
}

IplImage* Book::print(int p){
        if(img!=0 && p<dim) return img[p];
        else return NULL;
}
IplImage* Book::swap(int suc, double curtime){
        if(img[pos]->height!=img[suc]->height || img[pos]->width!=img[suc]->width) {
                printf("Immagini illustrate di grandezza diversa!!\n");
                return NULL;
        }
        int w=img[pos]->width;
        int h=img[pos]->height;
        double scale=abs(curtime-stime)/animtime;
        int x=w*(1.0-scale);
        int y=h*(1.0-scale);
        CvRect r1= cvRect(w-x,h-y,x,y);
        CvRect r2= cvRect(0,0,w-x,h-y);
        //IplImage* temp=cvCreateImage(cvSize(w,h), img[pos]->depth, img[pos]->nChannels);
        IplImage* temp=cvCloneImage(img[pos]);
        cvSetImageROI(img[pos],r1);
        cvSetImageROI(img[suc],r2);

        if( x>0 && y>0){
        cvSetImageROI(temp, cvRect(0,0,x,y));
        cvCopyImage(img[pos],temp);
        cvResetImageROI(temp);
        }

        if( w-x>0 && w-y>0) {
        cvSetImageROI(temp, cvRect(x,y,w-x,h-y));
        cvCopyImage(img[suc],temp);
        cvResetImageROI(temp);
        }

        cvResetImageROI(img[pos]);
        cvResetImageROI(img[suc]);
        return temp;
}
IplImage* Book::readInput(Hand_Mov hmov, double curtime){
        //INIZIO ANIMAZIONE
        if( stime==0 && hmov!=HAND_NOP) {
                stime=curtime;
                mov=hmov;
        }
        //ANIMAZIONE MOSTRO LA SUCCESSIVA
        if( stime!=0){
                if( stime+animtime<curtime ) { 
                stime=0;
                pos=mod(pos+mov,dim);
                mov=HAND_NOP;
                }
                else if( mov==HAND_TO_DX || mov==HAND_TO_SX) return swap(mod(pos+mov,dim),curtime);
        }
        return print(pos);
}