#include "tracking.h"

Tracking::Tracking(){
        //VARIABILI DI MOVIMENTO
        last=0;
        dim=20;
        frames=0;
        init= clock()/1000.00;
        buf_rect= (CvRect*) malloc( dim * sizeof(CvRect)); 
        memset( buf_rect, 0, dim*sizeof(CvRect));
        time_rect= (double*) malloc( dim * sizeof(double)); 
        memset( time_rect, 0, dim*sizeof(double));

        //VAR DI DEBUG
        f=fopen("result.txt","w");

        //VARIABILI DI CLASSE
        cannymask=0;
        cannyhand_mask=0;
        mask=0;
        hand_mask=0;
        size=cvSize(0,0);
        hand_size=cvSize(0,0);
        hperimeter=0;
        harea=0;
}
Hand_Mov Tracking::handCheckMovement( double interval, IplImage* dst ){
        //TEMPO IN SECONDI.MILLESIMI
        double timestamp=clock()/1000.0;
        frames++;
        const int distx=75;
        const int disty=30;
//		printf("FPS: %3.2f\n", timestamp);

        //INIZIALIZZAZIONE DEL BUFFER DI IMMAGINI
        if ( buf_rect==0 ) {
                buf_rect=(CvRect*) malloc(dim*sizeof(CvRect));
                memset(buf_rect, 0, dim*sizeof(CvRect));
        }

        //STAMPA SOLO QUELLI NELL'INTERVALLO
        double time=time_rect[mod(last-1,dim)];
        int range=last;
        if (time==0.0) return HAND_NOP;
        for (int i=mod(last-1,dim) ; i!=last; i=mod(i-1,dim)) {
                double dtime=time-interval-time_rect[i];
                if( dtime>0 || buf_rect[i].width==0 || buf_rect[i].height==0 ) {
                        range=i;
                        break;
                }
        }

        int cont=mod(last-(range+1), dim);
        if( cont==0) return HAND_NOP;
        CvPoint* p=(CvPoint*) malloc( cont*sizeof( CvPoint));


        //DEBUG STAMPA TUTTI I RETTANGOLI TROVATI
        int j=0;
        int avgx=0,avgy=0;
        for(int i=mod(last-1,dim) ; i!=range; i=mod(i-1,dim))  {
                p[j].x=buf_rect[i].x;
                p[j].y=buf_rect[i].y;
                avgx+=p[j].x;
                avgy+=p[j].y;
                //cvRectangle( dst,p[j],cvPoint(50,50),cvScalar(0,255,0,0),1);
                cvRectangleR( dst,buf_rect[i],cvScalar(0,255,0,0),1);
                j++;
        }

        //STABILISCO LA DIREZIONE
        avgx=avgx/(double)cont;
        avgy=avgy/(double)cont;
        int dir=0;
        if( p[0].x>avgx) dir=1;
        else dir=-1;

        //CHECK X E Y NORMALIZZO QUELLI ERRATI
        int seq=1;
        double incx=0;
        int totalx=0;
        double seqmatch=0;
        for(int i=0 ; i<cont-1; i++)  {
                if( !(abs(p[i].y-p[i+1].y)<disty) 
                        || !(dir==1 && p[i+1].x+incx < p[i].x && p[i+1].x+distx > p[i].x) 
                        && !(dir==-1 && p[i+1].x-incx > p[i].x && p[i+1].x-distx < p[i].x) 
                        ){
                        cvLine(dst, p[i],p[i+1], cvScalar(0,0,255),3);
                        memcpy( &p[i+1], &p[i], sizeof(CvPoint));
                }
                else {
                        //incx=abs(p[i].x-p[i+1].x)*0.50;
                        totalx+=abs(p[i].x-p[i+1].x);
                        seq++;
                        cvLine(dst, p[i],p[i+1], cvScalar(255,255,0),3);
                }	
        }
        free( p);
        seqmatch=seq/(double)cont;
        printf("#rect %d  -- #sequenza  %d    valore:%f\n", cont, seq, seqmatch );
        if( seq >= 5 && seqmatch>0.65 && totalx>size.width*0.30) {
                memset(buf_rect, 0, dim* sizeof(CvRect));
                memset(time_rect, 0 , dim*sizeof(double));
                if( dir==1) return HAND_TO_DX;
                if( dir==-1) return HAND_TO_SX;
        }
return HAND_NOP;
}
IplImage* Tracking::handStartup ( IplImage* mask, IplImage* hand_mask) {
        if( mask->nChannels!=1) return 0; 
        this->mask=mask;
        this->hand_mask=hand_mask;
        this->size=cvGetSize(mask);
        this->hand_size= cvGetSize( hand_mask);
        this->depth=mask->depth;
        this->nChannels=mask->nChannels;
        cvReleaseImage( &cannymask);
        cvReleaseImage( &cannyhand_mask);
        cannymask=0;
        cannyhand_mask=0;
        cannymask= cvCreateImage( size, depth, nChannels);
        cannyhand_mask=cvCreateImage( hand_size, depth, nChannels);
        cvCanny(mask, cannymask, 30.0,250.0, 3);
        cvCanny(hand_mask, cannyhand_mask, 30.0,250.0, 3);

        //CALCOLO DELLE CARATTERISTICHE MASCHERA DELLA MANO
        hperimeter=cvCountNonZero(cannyhand_mask);
        harea= cvCountNonZero( hand_mask);
        return NULL;
}
int Tracking::handCheckUp() {
        if( cannymask==0 || cannyhand_mask== 0) return 0;
        if( mask==0 || hand_mask==0) return 0;
        if ( size.width==0 || hand_size.width==0) return 0; 
        if( hperimeter==0 || harea==0) return 0;
        return 1;
}
//LA PROCEDURA DEVE CONFRONTARE LA MANO CON UN TEMPLATE
IplImage*  Tracking::handRecognition(IplImage* src,IplImage* mask, IplImage* hand_templ, IplImage* hand_mask){
// FARE CONTROLLI IMMAGINE!!
        if( mask->width!=size.width ||  mask->height!=size.height || mask->depth!=depth) return NULL; 
        if( !handCheckUp() ) return NULL; 

//CONVERTIRE DA 8 bit a 32 bit
//cvCvtScale( src, src32, 1/255.0);

IplConvKernel* element = cvCreateStructuringElementEx( 5,5,3,3, CV_SHAPE_RECT , 0 );
//Dilation adds a layer on, and returns things to the correct size.
cvDilate(mask,mask,element,1);
cvErode(mask,mask,element,1);

 CvMemStorage* stor= cvCreateMemStorage( 0);
 CvSeq* cont= cvCreateSeq(CV_SEQ_ELTYPE_CONNECTED_COMP, sizeof(CvSeq), sizeof(CvConnectedComp), stor);

 IplImage* temp_mask=cvCloneImage( mask);
 cvFindContours( temp_mask , stor, &cont, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

 CvRect r=contornArea( src, stor, cont);
 if( r.width!=0 & r.height!=0){
         double time= clock()/1000.0;
         time_rect[last]=time;
         buf_rect[last]=r;
         last=(last+1)%dim;
 }
 return src;
}
//FORSE ME LA CAVO CON IL PUNTO PIU ALTO A SX E PUNTO PIU ALTO A DX
CvRect Tracking::contornArea( IplImage* add_rect, CvMemStorage* stor, CvSeq* cont){
        int width=hand_mask->width;
        int heigth=hand_mask->height;
        int area = width*heigth;	
        CvSeq* head=cont;
        double diffbest=-1;
        CvRect hrect=cvRect(0,0,0,0);
        double ratioM=0,ratioP=0,ratioA=0;

        int seq=0;
        for( ; cont!=NULL; ) {
                CvRect r= ((CvContour*)cont)->rect;
                CvSeq* s=cont;
                cont=cont->h_next;
                if ( r.width*r.height>area*0.5){ 
                        seq++;
                }
                else {
                        cvClearSeq(s);
                        if( s==head ) head=cont;
                }
        }

        //IMPLEMENTARE CONTROLLI BORDI IMMAGINI
        cont=head;
        for(int s=0; s<seq ; s++) {
                if(cont->total==0) {
                        continue;
                        cont=cont->h_next;
                }
                CvPoint* p=(CvPoint *) cvGetSeqElem( cont, 0);
                int minx=p[0].x, maxx=p[0].x;
                int y1=0, y2=0;

                for( int i=0; i<cont->total-1; i++) {
                        if ( p[i].x >maxx) {
                                maxx= p[i].x;
                                y1=p[i].y;
                        }
                        if ( p[i].x <minx) {
                                minx= p[i].x;
                                y2=p[i].y;
                        }
                        cvLine( add_rect, p[i], p[i+1] , cvScalar(0,s*10%255,0,0), 4, 8) ; 
                }

                int sx,sy;
                int dx,dy;

                if ( maxx+4>=size.width) dx=size.width-hand_size.width-4;
                else dx=maxx-hand_size.width-4;
                if( y1-(heigth*0.55) <=0) dy=0; 
                else dy=y1-(heigth*0.55);

                //DEBUG
                cvRectangle( add_rect, cvPoint( maxx-3,y1-3),cvPoint(maxx+3,y1+3), cvScalar( 255,0,0,0), 3);
                cvRectangle( add_rect, cvPoint( dx-3,dy-3),cvPoint(dx+3,dy+3), cvScalar(255 ,0,255,0), 3);

                if( minx<4) sx=0;
                else sx=minx-4;
                if( y2-(heigth*0.55) <=0) sy=0; 
                else sy=y2-(heigth*0.55);

                //FARE IL CONTROLLO SULLE POSIZIONI VALIDE O MENO
                CvRect srect= cvRect(sx, sy, width+8, heigth*1.1 ) ;
                CvRect drect= cvRect(dx, dy, width+8, heigth*1.1 ) ;
                //printf("RECT %d    %d    %d    %d\n",r.x,r.y,r.width,r.height);

                //RETTANGOLO SINISTRO
                cvSetImageROI( cannymask, srect);	
                cvSetImageROI (mask, srect);
                IplImage* copy=cvCloneImage( mask);
                IplImage* match=cvCreateImage( hand_size, depth, nChannels);
                cvResize( copy, match , 1);
                cvCmp( match , hand_mask, match, CV_CMP_EQ);
                double sx_matching = cvCountNonZero(match)/((double) area);
                double sx_perimeterROI= cvCountNonZero(cannymask)/((double)hperimeter);
                double sx_areaROI= cvCountNonZero(mask)/((double)harea);
                cvReleaseImage(&match);
                cvReleaseImage(&copy);
                cvResetImageROI( cannymask);
                cvResetImageROI( mask);

                //RETTANGOLO DESTRO
                cvSetImageROI( cannymask, drect);	
                cvSetImageROI (mask, drect);
                copy=cvCloneImage( mask);
                match=cvCreateImage( hand_size, depth, nChannels);
                cvResize( copy, match , 1);
                cvCmp( match , hand_mask, match, CV_CMP_EQ);
                double dx_matching = cvCountNonZero(match)/((double) area);
                double dx_perimeterROI= cvCountNonZero(cannymask)/((double)hperimeter);
                double dx_areaROI= cvCountNonZero(mask)/((double)harea);
                cvResetImageROI( cannymask);
                cvResetImageROI( mask);
                cvReleaseImage(&match);
                cvReleaseImage(&copy);

                //COEFFICENTI DI MATCHING DELLE AREE
                double sx_val= sx_areaROI/sx_perimeterROI;
                double dx_val= dx_areaROI/dx_perimeterROI;
                double sx_diffval=(1.0-sx_matching)*((abs(1.0-sx_perimeterROI))+abs(1.0-sx_areaROI));
                double dx_diffval=(1.0-dx_matching)*((abs(1.0-dx_perimeterROI))+abs(1.0-dx_areaROI));

                if(diffbest==-1) diffbest=10;
                if( sx_diffval < diffbest && sx_matching>0.52 && sx_val>0.55 && sx_val<1.05 && sx_areaROI>0.6 && sx_perimeterROI>0.65 ) {
                        diffbest=sx_diffval;
                        ratioM=sx_matching;
                        ratioA=sx_areaROI;
                        ratioP=sx_perimeterROI;
                        memcpy( &hrect , &srect, sizeof(CvRect));
                }
                if( dx_diffval < diffbest && dx_matching>0.5 && dx_val>0.5 && dx_val<1.05 && dx_areaROI>0.6 && dx_perimeterROI>0.65 ) {
                        diffbest=dx_diffval;
                        ratioM=dx_matching;
                        ratioA=dx_areaROI;
                        ratioP=dx_perimeterROI;
                        memcpy( &hrect , &drect, sizeof(CvRect));
                }
                //fprintf(f,"SCARTATO Position %d  %d   %d   %d\nMatching %2.4f - Area %2.4f - Perimetro %2.4f  -> Risultato %4.4f\n\n", drect.x,drect.y,drect.width,drect.height, dx_matching, dx_areaROI, dx_perimeterROI, dx_diffval);
                cont=cont->h_next;
        }
        fprintf(f,"Position %d  %d   %d   %d\nMatching %2.4f - Area %2.4f - Perimetro %2.4f  -> Risultato %4.4f\n", hrect.x,hrect.y,hrect.width,hrect.height, ratioM, ratioA, ratioP, diffbest);
        fflush(f);
        cvRectangleR( add_rect, hrect, cvScalar(0,255,0,0), 3);

        cvReleaseMemStorage(&stor);
        return hrect;
}
int Tracking::handTemplateSave( char* file, IplImage* src, CvRect* hand){
        cvSetImageROI( src, *hand);
        int result= cvSaveImage( file, src);
        cvResetImageROI( src);
        return result;
}
//GENERA I TEMPLATE DI AREA MASSIMA!!!
int Tracking::handTemplate( IplImage* src, IplImage* mask, IplImage* dst, CvRect* handROI){
//FARE CONTROLLI IMMAGINE
        CvMemStorage* stor = cvCreateMemStorage(0);
        CvSeq* cont;// = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint) , stor);
        IplImage* copy=cvCloneImage( mask); 

        cvCvtColor( mask, dst, CV_GRAY2RGB);
        cvFindContours( copy, stor, &cont,sizeof(CvContour),CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
        if( contornAreaMax( dst,stor,cont,1.2, handROI) ) {
                return 1;
        }
        return 0;
}
int Tracking::contornAreaMax( IplImage* add_rect, CvMemStorage* stor, CvSeq* cont, double ratio, CvRect* rect){
        int maxArea=1000;
        double hmax=0.0, hmin=0.0;
        double hmed=0.0;
        CvPoint p1;
        CvPoint p2;
        memset( &p1, 0, sizeof( CvPoint));
        memset( &p2, 0, sizeof( CvPoint));
        for(;cont;cont = cont->h_next)
        {
      CvRect r = ((CvContour*)cont)->rect;
                  hmed= r.width/((double)ratio);
                  //SCARTO IL BRACCIO PARTO DA SX IN ALTO VS DESTRA BASSO
                  if( r.width*r.height> maxArea )
      {
                          maxArea=r.height * r.width;
                          p1=cvPoint( r.x, r.y);
                          p2=cvPoint(r.x + r.width, r.y + hmed );
      }
        }
        rect->width=p2.x-p1.x;
        rect->height=p2.y-p1.y;
        rect->x=p1.x;
        rect->y=p1.y;
        cvRectangleR( add_rect, *rect ,CV_RGB(255,0,0), 1, CV_AA,0);
        // free memory
        cvReleaseMemStorage(&stor);
        return 1;
}