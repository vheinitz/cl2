/*V. Heinitz, 2012-04-05
Very experimental code with many copy&pastes and unused functions.
Among other things implements:
-calculation algorithms for focus-evaluation
-Simple pre-classification 
*/
#if 0
#include "imgproc.h"
#include <QRgb>
#include <QPainter>
#include <QThreadPool>
#include <QDir>
#include <QtCore/qmath.h>
#include <QLabel>
#include <QCryptographicHash>
#include <QMutexLocker>
#include <QTransform>

#pragma warning( push )
#pragma warning( disable : 4996 ) // disable unsafe fopen, strcpy in OpenCV
#include <cv.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <vector>
#pragma warning( pop )

#include <QDir>
#include <QFile>
#include <QFileInfo>
using namespace cv;

IplImage * createGreyFromQImage(QImage img, int ch=1);
QImage IplImage2QImage(IplImage *iplImg);



QImage ImgProc::analyseStripe(QImage qi)
{
	 Mat src = toMat(qi);


	 Mat mask = src.clone();

	 int window = src.cols / 10;
	 for (int c=0; c<=src.cols-(window+1); c+=window)
	 {
		 Mat roi = mask( Rect( c,0,window,src.rows ) );
		 threshold(roi, roi, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	 }

	  int erosion_size = 1;
	  Mat element = getStructuringElement( MORPH_RECT,
										   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
										   Point( erosion_size, erosion_size ) );

	  //erode( mask, mask, element );
	  //dilate( mask, mask, element );

	 vector<int> graph( src.cols );
	 for (int c=0; c<src.cols-1; c++)
	 {
		 Mat roi = src( Rect( c,0,1,src.rows ) );
		 graph[c] = 255 - int(mean(roi)[0]);
	 }
	 int n=graph.size();

	 vector<int> mask_graph( src.cols );
	 for (int c=0; c<mask.cols; c++)
	 {
		 Mat roi = mask( Rect( c,0,1,mask.rows ) );
		 int v= int(mean(roi)[0]);
		 mask_graph[c-0] = (v > 0?255:0); // BW-edges
	 }

	 vector<int> bars( src.cols );
	 

	 
	 int lastval=0;
	 int barval=0;
	 int cnt=0;

	 int base_lastval=0;
	 int base_val=0;
	 
	 int base_cnt=0;

	 vector<int> bar_values( n );
	 vector<int> base_values( n );
	 for (int i=0; i<n; i++)
	 {
		 bar_values[i]=0;
		 base_values[i]=0;
		 if ( mask_graph[i]==0 )  
		 {		
			if (base_cnt)
			{
				base_values[i]= base_val / base_cnt;
			}
			base_val=0;
			base_cnt=0;

			barval += graph[i];
			cnt++;
		 }
		 else
		 {
			if (cnt)
			{
				bar_values[i]= barval / cnt;
			}
			barval=0;
			cnt=0;

			base_val += graph[i];
			base_cnt++;
		 }
	 }

	// imshow("mask", mask);


	 int bx1=-1;
	 int by1=-1;
	 for (int c=0; c<n; c++)
	 {
		 if ( base_values[c] !=0 )
		 {
			 bx1=c;
			 by1 = base_values[c];
			 break;
		 }
	 }
	 
	 int bx2=-1;
	 int by2=-1;
	 for (int c=n-1; c>0; c--)
	 {
		 if ( base_values[c] !=0 )
		 {
			 bx2=c;
			 by2 = base_values[c];
			 break;
		 }
	 }
	 

	// int n=graph.size()-1;
	 int PK1x=-1;
	 int PK1y=-1;
	 for (int c=0; c<n; c++)
	 {
		 if ( bar_values[c] !=0 )
		 {
			 PK1x=c;
			 PK1y = bar_values[c];
			 break;
		 }
	 }
	 

	 int PK2x=-1;
	 int PK2y=-1;
	 for (int c=n-1; c>0; c--)
	 {
		 if ( bar_values[c] !=0 )
		 {
			 PK2x=c;
			 PK2y = bar_values[c];
			 break;
		 }
	 }
	  
	 double t = double((PK1y-PK2y)) / double((PK2x-PK1x));
	 double tb = double(by1-by2) / double(bx2-bx1);

	 for (int c=0; c<n; c++)
	 {
		 if ( bar_values[c] !=0 )
		 {
			 double x=c;
			 double y = bar_values[c];
			 double yc = (x / (PK2x-PK1x)) * (PK1y-PK2y);
			 bar_values[c] = bar_values[c] + yc;
		 }
	 }

	 ///
	 int cx1=-1;
	 int cy1=-1;
	 bool isCutoff=0;
	 for (int c=0; c<n; c++)
	 {
		 if ( base_values[c] !=0 )
		 {
			 if( isCutoff )
			 {
				 cx1=c;
				 cy1 = base_values[c];
				 break;
			 }
			 else
				 isCutoff=1;
		 }
	 }
	 
	 int cx2=PK2x;
	 int cy2=PK2y/2;
	 double tc = (t+tb)/2;//double(cy1-cy2) / double(cx2-cx1);
	 ///

	 for (int c=0; c<n; c++)
	 {
		 if ( bar_values[c] !=0 )
		 {
			 if( bar_values[c] < PK1y )
			 {
				 double y = bar_values[c] - (bx2-c) * tc;
				 bar_values[c] = y;
			 }
		 }
	 }


	 Mat mgraph(  260, src.cols+10, CV_8UC3); 
	 for (int c=0; c<n-1; c++)
	 {
		 line( mgraph, Point(c+5,0), Point(c+5,255-graph[c]), Scalar(255,0,0), 1, CV_AA);	
		 line( mgraph, Point(c+5,259), Point(c+5,259-bar_values[c]), Scalar(0,255,255), 5, CV_AA);	 
	 }
	 //imshow("source", src);
	 //imshow("mgraph", mgraph);

	 return toQImage(mgraph);
}




Mat ImgProc::toMat( QImage src, ChannelMode in, ChannelMode out )
{
	//TODO evaluate in out settings
	cv::Mat tmp(src.height(),src.width(),CV_8UC4,(uchar*)src.bits(),src.bytesPerLine());
    cv::Mat result;
    cvtColor(tmp, result,CV_BGRA2RGB);
	
	Mat channels[3];
	split(result,channels); 
    return channels[1];	
}

QImage ImgProc::toQImage( const cv::Mat &src )
{ 
	return IplImage2QImage( new IplImage(src) );
}

void ImgProc::rectify(QImage & img, QList<QPoint> corners, QRectF * br)
{
	Mat src=toMat( img );
	Mat transformed = Mat::zeros(src.rows, src.cols, CV_8UC3);

	Point2f quad_pts[4];
	Point2f squre_pts[4];
	
	QPolygonF pol;
	foreach ( QPoint p, corners )
	{
		pol << p;
	}

	quad_pts[0] = Point2f(corners.at(0).x(), corners.at(0).y());
	quad_pts[1] = Point2f(corners.at(1).x(), corners.at(1).y());
	quad_pts[2] = Point2f(corners.at(2).x(), corners.at(2).y());
	quad_pts[3] = Point2f(corners.at(3).x(), corners.at(3).y());
	
	QRectF r = pol.boundingRect();
	if (br)
		*br = r;

	squre_pts[0]=Point2f(r.x(),r.y());
	squre_pts[1]=Point2f(r.x()+r.width(),r.y());	
    squre_pts[2]=Point2f(r.x(),r.y()+r.height());    
	squre_pts[3]=Point2f(r.x()+r.width(),r.y()+r.height());

	try
	{
		Mat transmtx = getPerspectiveTransform(quad_pts,squre_pts);
		transformed = Mat::zeros(src.rows, src.cols, CV_8UC3);
		warpPerspective(src, transformed, transmtx, src.size());
		img = toQImage( transformed );
	}
	catch(std::exception e)
	{
	}
}

void ImgProc::storeImageTask(QString path, QImage img, int beautify, TImageMap bestImages, QObject *callback)
{    
	StoreImageThread *tmpImgTask = new StoreImageThread(path, img, beautify, bestImages) ;
	if( callback )
		QObject::connect( tmpImgTask, SIGNAL(fileSaved(QString,QStringList)), callback, SLOT(processFileSaved(QString,QStringList)));

	StoreImageThread::_StoreImageTasks.append(tmpImgTask);
    QThreadPool::globalInstance()->start( tmpImgTask );
}

bool ImgProc::storeImageTaskBusy(int & readypct)
{
	bool result=false;
	QMutableListIterator<StoreImageThread*> i(StoreImageThread::_StoreImageTasks);
	int todo=0;
	int done=0;
	while (i.hasNext()) 
	{
		StoreImageThread* n= i.next();
		todo+=n->numberToProcess();
		done+=n->numberOfFinished();
		if (n->isBusy() )
		{
			result = true;
		}
		else
		{
			i.remove();		
		}
	}
	
	if ( todo == 0 )
		readypct = 100;
	else if(todo > 0)
		readypct = (done * 100) / todo;
	else
		readypct = 0; //TODO log Error

	return result;
}


void ImgProc::reduceBgNoise(QImage & img)
{
    PDImageInfo iminf;
    getImageInfo(img, iminf);
    reduceBackgroundNoise(img, iminf._bgLevel1 );
}

void ImgProc::sharpen(QImage & img)
{
	IplImage* cvimg = createGreyFromQImage( img );
	if ( !cvimg ) return;

	IplImage* gsimg = cvCloneImage(cvimg );
	IplImage* dimg = cvCreateImage( cvGetSize(cvimg), IPL_DEPTH_8U, 1 );
	IplImage* outgreen = cvCreateImage( cvGetSize(cvimg), IPL_DEPTH_8U, 3 );
	IplImage* zeroChan = cvCreateImage( cvGetSize(cvimg), IPL_DEPTH_8U, 1 );
	cvZero(zeroChan);

	cv::Mat smat( gsimg, false );
	cv::Mat dmat( dimg, false );

	cv::GaussianBlur(smat, dmat, cv::Size(0, 0), 3);
	cv::addWeighted(smat, 1.5, dmat, -0.5 ,0, dmat);
	cvMerge( zeroChan, dimg, zeroChan, NULL, outgreen);

	img = IplImage2QImage( outgreen );
	cvReleaseImage( &gsimg );
	cvReleaseImage( &cvimg );
	cvReleaseImage( &dimg );
	cvReleaseImage( &outgreen );
	cvReleaseImage( &zeroChan );
}


void ImgProc::beautifyImage(QString img)
{

	QDir().mkdir("TEMP");
	QDir().remove("TEMP/tmpimg.png");
    QImage im(img);
    PDImageInfo iminf;
    getImageInfo(im, iminf);
    reduceBackgroundNoise(im, iminf._bgLevel1 );
    im.save(img);
	QFile::copy( img, "TEMP/tmpimg.png" );

	IplImage* simg = cvLoadImage("TEMP/tmpimg.png", 1);
	if ( !simg ) 
		return;

	IplImage* gsimg = cvCreateImage( cvGetSize(simg), IPL_DEPTH_8U, 1 );
	IplImage* dimg = cvCreateImage( cvGetSize(simg), IPL_DEPTH_8U, 1 );
	IplImage* outgreen = cvCreateImage( cvGetSize(simg), IPL_DEPTH_8U, 3 );
	IplImage* zeroChan = cvCreateImage( cvGetSize(simg), IPL_DEPTH_8U, 1 );
	cvZero(zeroChan);
	cvSplit( simg, 0, gsimg,0,0 );
	//cvEqualizeHist(gsimg, dimg);

	cv::Mat smat( gsimg, false );
	cv::Mat dmat( dimg, false );

	cv::GaussianBlur(smat, dmat, cv::Size(0, 0), 3);
	cv::addWeighted(smat, 1.5, dmat, -0.5 ,0, dmat);
	cvMerge( zeroChan, dimg, zeroChan, NULL, outgreen);

	cvSaveImage( QS2CS(img), outgreen );
	cvReleaseImage( &gsimg );
	cvReleaseImage( &simg );
	cvReleaseImage( &dimg );
	cvReleaseImage( &outgreen );
	cvReleaseImage( &zeroChan );
}

bool ImgProc::detectShift(QImage &img1, QImage &img2, int _tileSize, int &dx, int &dy )
{
	IplImage *src, *templ, *ftmp[6]; // ftmp will hold results
	int i;

	QImage tile = img1.copy(img1.width()/2-( _tileSize/2 ), img1.height()/2-( _tileSize/2 ), _tileSize, _tileSize );

	templ = createGreyFromQImage( tile , 1);
	src = createGreyFromQImage( img2 , 1);

	// Allocate Output Images:
	int iwidth = src->width - templ->width + 1;
	int iheight = src->height - templ->height + 1;
	for(i = 0; i < 6; ++i){
		ftmp[i]= cvCreateImage( cvSize( iwidth, iheight ), 32, 1 );
	}

	// Do the matching of the template with the image
	for( i = 0; i < 6; ++i ){
		cvMatchTemplate( src, templ, ftmp[i], i );
		cvNormalize( ftmp[i], ftmp[i], 1, 0, CV_MINMAX );
	}

	// DISPLAY
	//cvNamedWindow( "Template", 0 );
	//cvShowImage( "Template", templ );
	//cvNamedWindow( "Image", 0 );
	//cvShowImage( "Image", src );
	/*
	cvNamedWindow( "SQDIFF", 0 );
	cvShowImage( "SQDIFF", ftmp[0] );
	cvNamedWindow( "SQDIFF_NORMED", 0 );
	cvShowImage( "SQDIFF_NORMED", ftmp[1] );
	cvNamedWindow( "CCORR", 0 );
	cvShowImage( "CCORR", ftmp[2] );
	cvNamedWindow( "CCORR_NORMED", 0 );
	cvShowImage( "CCORR_NORMED", ftmp[3] );
	*/
	//cvNamedWindow( "COEFF", 0 );
	//cvShowImage( "COEFF", ftmp[4] );
	//cvNamedWindow( "COEFF_NORMED", 0 );
	//cvShowImage( "COEFF_NORMED", ftmp[5] );

	cvReleaseImage (&src );
	cvReleaseImage (&templ );

	for( i = 0; i < 6; ++i ){
		cvReleaseImage (&ftmp[i] );
	}
	return true;
}

double ImgProc::compareHistogramm(QImage &imgo1, QImage &imgo2, int meth, QRect r)
{
	QImage img1,img2;
	img2 = imgo2;
	if( r.isNull() )
	{
		img1 = imgo1;
		
	}
	else
	{
		img1 = imgo1.copy(r);
		//img2 = imgo2.copy(r);
	}

	int hist_size = 256;
	float range_0[]={0, (float) hist_size};
	float* ranges[] = { range_0 };

	IplImage * i1 = createGreyFromQImage( img1 , 1);	
	CvHistogram* h1 = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
	cvCalcHist ( &i1 , h1 , 0, NULL );
	cvReleaseImage (&i1 );	
	cvNormalizeHist ( h1 , 1.0) ;			

	IplImage * i2 = createGreyFromQImage( img2 , 1);	
	CvHistogram* h2 = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
	cvCalcHist ( &i2 , h2 , 0, NULL );
	cvReleaseImage (&i2 );
	cvNormalizeHist ( h2 , 1.0) ;			

	double result =  cvCompareHist ( h1 , h2 , meth );
	cvReleaseHist (& h1 );
	cvReleaseHist (& h2 );
	return result;
}


void kMeans(QList<QPoint> &dataVector, const int clusterCount,
	QList < QList<QPoint> > &clusterContainer)
{
    /*
      * Pre: "dataVector" the data to be clustered by K-Means
      * "clusterCount" how many clusters you want
      *
      * Post: "classContainer" I pack the points with the same cluster into vector, so it
      * is a vetor of vector
      */
     
     
    int dataLength = dataVector.size();
     
     
    // Put data into suitable container
    CvMat* points = cvCreateMat(dataLength, 1, CV_32FC2);
    CvMat* clusters = cvCreateMat(dataLength, 1, CV_32SC1 );
     
    for (int row = 0; row < points->rows; row++) {
    float* ptr = (float*)(points->data.ptr + row*points->step);
    for (int col = 0; col < points->cols; col++) {
    *ptr = static_cast<float>(dataVector[row].x());
    ptr++;
    *ptr = static_cast<float>(dataVector[row].y());
    }
    }
     
    // The Kmeans algorithm function (OpenCV function)
    cvKMeans2(points, clusterCount, clusters, cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 1, 2));
     
    // Pack result to 'classContainer', each element in 'classContainer' means one cluster,
    // each cluster is one vector<CvPoint> contain all points belong to this cluster
    int clusterNum;
    QList<QPoint> tempClass;
     
    for (int i = 0; i < clusterCount; i++) {
    tempClass.clear();
     
    for (int row = 0; row < clusters->rows; row++) {
     
     
    float* p_point = (float*)(points->data.ptr + row*points->step);
    int X = static_cast<int>(*p_point) ;
    p_point++;
    int Y = static_cast<int>(*p_point);
     
    clusterNum = clusters->data.i[row];
     
    if (clusterNum == i)
    tempClass.push_back(QPoint(X, Y));
     
    }
     
    clusterContainer.push_back(tempClass);
     
    }
     
    // Remove empty cluster
    for (vector< vector<CvPoint> >::size_type i = 0; i < clusterContainer.size(); ++i) {
     
    bool isEmpty = clusterContainer[i].empty();
     
    if (isEmpty) {
    QList< QList<QPoint> >::iterator iter = clusterContainer.begin();
    iter = iter + i;
    clusterContainer.erase(iter);
    i = i - 1;
    }
    }
     
    cvReleaseMat(&points);
    cvReleaseMat(&clusters);
     
     
    }

int ImgProc::findLocMaxima( QImage & img, int step, int minTh, QList<QRect> * positions )
{
	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/2, img.height()/2) );

	//IplImage *orig = createGreyFromQImage(img);
	
	//IplImage* work = cvCreateImage( cvSize(orig->width/2,orig->height/2), orig->depth, orig->nChannels );
	IplImage* work =  createGreyFromQImage(img);
	//cvPyrDown(orig,work);
	//cvShowImage("work", work);



	CvRect roi;
	roi.height = step;
	roi.width = step;
	struct MaxPoint{ QPoint _pos; double _max; MaxPoint( QPoint pos = QPoint(), double max=0 ):_pos(pos),_max(max){} };
	QList<MaxPoint> maxima;
	
	CvScalar s = cvAvg( work );
	double lastMax=0;
	QPoint lastMaxPos;
	
	for ( int x=0; x <=work->width-step; x+=step/2 )
	{
		
		for ( int y=0; y <=work->height-step; y+=step/2 )
		{
			
			roi.x=x;
			roi.y=y;
			cvSetImageROI( work, roi );
			CvPoint pmax;
			CvPoint pdummy;
			double maxvalue;
			cvMinMaxLoc(work,0,&maxvalue,0,&pmax);
			cvResetImageROI( work );
			bool replaced=false;
			QPoint p( x+pmax.x,y+pmax.y );

			if ( maxvalue > s.val[0] &&  maxvalue >= minTh ) //threshold ok
			{								
				for ( QList<MaxPoint>::iterator it = maxima.begin(); it != maxima.end(); ++it )
				{
					int ms = maxima.size();
					double pmax = it->_max;
					QPoint pp = it->_pos;
					if ( qAbs( it->_pos.x() - p.x() ) <= step && qAbs( it->_pos.y() - p.y() ) <= step  )
					{
						if ( it->_max <  maxvalue )
						{
							it->_pos = p;
							it->_max = maxvalue;							
						}
						replaced = true;
						break;
					}					
				}
				
				if (!replaced)
					maxima.append( MaxPoint(p,maxvalue) );	
				
			}
			
		}
		
	}


	if (positions)
	{
		foreach( MaxPoint p, maxima )
		{

			positions->append( QRect(QPoint(p._pos.x()*2,p._pos.y()*2),QSize(5,5)) );
		}
	}
	

	//cvReleaseImage (&orig );
	cvReleaseImage (&work );

	return maxima.size();

}

QPair<double, double> ImgProc::getMeanAndDev ( const QList<double> list )
{
	QPair<double, double> ret(0,0);
	double stdDev=0;
	double mean=0;
	foreach( double v, list )
	{
		mean+=v;
	}
	mean /= list.size();
	ret.first = mean;
	foreach(  double v, list )
	{
		stdDev+= ( v - mean )*( v - mean );
	}
	stdDev /= list.size()-1;
	stdDev = qSqrt( stdDev );
	ret.second = stdDev;
	return ret;
}

int ImgProc::removeInhomogeniousAreas( QImage & img )
{
	IplImage *orig = createGreyFromQImage(img);	
	IplImage* work = cvCreateImage( cvSize(orig->width/2,orig->height/2), orig->depth, orig->nChannels );
	cvPyrDown(orig,work);
	cvNormalize(work,work,0,250,CV_MINMAX);


	int stepx = work->width / 4;
	int stepy = work->height / 4;
	CvRect roi;
	roi.height = stepy;
	roi.width = stepx;
	typedef QPair<CvRect,double> TRegionEvg; 
	QList< TRegionEvg > avgs;
	
	CvScalar s = cvAvg( work );
	CvScalar avg;
	for ( int x=0; x <=work->width-stepx; x+=stepx/2 )
	{
		for ( int y=0; y <=work->height-stepy; y+=stepy/2 )
		{
			roi.x=x;
			roi.y=y;
			cvSetImageROI( work, roi );
			avg = cvAvg(work);
			avgs.append( TRegionEvg(roi, avg.val[0]) );
			cvResetImageROI( work );
		}
	}

	double stdDev=0;
	double mean=0;
	foreach( TRegionEvg ra, avgs )
	{
		mean+=ra.second;
	}
	mean /= avgs.size();
	avg.val[0] = mean;
	foreach( TRegionEvg ra, avgs )
	{
		stdDev+= ( ra.second - mean )*( ra.second - mean  );
	}
	stdDev /= avgs.size()-1;
	stdDev = qSqrt( stdDev );
	

	double stdDevPc=(stdDev / mean) * 100;

	if ( stdDevPc > 10 )
	{
		foreach( TRegionEvg ra, avgs )
		{
			if( ra.second > mean + stdDev || ra.second < mean - stdDev )
			{
				cvSetImageROI( work, ra.first );
				//cvSet(work,avg);
				cvZero(work);
				cvResetImageROI( work );
			}
		}
	}
	cvPyrUp(work,orig);
	img = IplImage2QImage(orig);

	//Calculating mask area: todo: add inhomogenious rects instead doing it by pixel calculation
	int area = orig->width * orig->height;
	area -= cvCountNonZero(orig);

	if ( area < stepx*stepy ) //ignoring "could" pixels
		area=0;
	
	cvReleaseImage (&orig );
	cvReleaseImage (&work );

	return area; //of inhomogenious regions

}

QMap<QByteArray,int> ImgProc::getSequenceHistogram( QImage & img, int seqLen, int levels )
{
	IplImage *orig = createGreyFromQImage(img);	
	IplImage* tmpi = cvCreateImage( cvSize(orig->width/2,orig->height/2), orig->depth, orig->nChannels );
	IplImage* tmpii = cvCreateImage( cvSize(orig->width/4,orig->height/4), orig->depth, orig->nChannels );
	IplImage* work = cvCreateImage( cvSize(orig->width/8,orig->height/8), orig->depth, orig->nChannels );
	cvPyrDown(orig,tmpi);
	cvPyrDown(tmpi,tmpii);
	cvPyrDown(tmpii,work);


	cvNormalize(work,work,0,levels-1,CV_MINMAX);
	QImage tmp = IplImage2QImage(work);

	cvNormalize(work,work,0,250,CV_MINMAX);
	//cvShowImage("adasda", work );

	
	cvReleaseImage (&tmpi );
	cvReleaseImage (&tmpii );
	cvReleaseImage (&orig );
	cvReleaseImage (&work );

	QMap<QByteArray,int> ret;
	QByteArray ba(seqLen,Qt::Uninitialized);
	for (int y =0; y < tmp.height(); y++)
    {			        
		QRgb * pix = (QRgb *)tmp.scanLine(y);
        for (int x =0; x < tmp.width() - seqLen; ++x)
        {				
			for (int i=0; i< seqLen; ++i)
				ba[i] = qGreen( *(pix + i) );
			ret[ba]++;
			pix++;
		}
	}
	return ret;
}

void ImgProc::normalize( QImage & img )
{
	IplImage *orig = createGreyFromQImage(img);	
	IplImage* work = cvCreateImage( cvSize(orig->width/2,orig->height/2), orig->depth, orig->nChannels );
	cvPyrDown(orig,work);


	int stepx = work->width / 10;
	int stepy = work->height / 10;
	CvRect roi;
	roi.height = stepy;
	roi.width = stepx;
	
	for ( int x=0; x <=work->width-stepx; x+=stepx )
	{
		for ( int y=0; y <=work->height-stepy; y+=stepy )
		{
			roi.x=x;
			roi.y=y;
			cvSetImageROI( work, roi );
			cvNormalize(work,work,0,250,CV_MINMAX);
			cvResetImageROI( work );
		}
	}

	cvPyrUp(work,orig);
	cvNormalize(orig,orig,0,250,CV_MINMAX);
	img = IplImage2QImage(orig);

	cvReleaseImage (&orig );
	cvReleaseImage (&work );

}

//TODO: Experimental code for pattern matching. Move in separate function
QList<QRect> ImgProc::findItems( QImage & imgIn, QImage tmplIn, int meth )
{
	IplImage * timg = createGreyFromQImage(imgIn);	
	IplImage * ttmpl = createGreyFromQImage(tmplIn);

	cv::Mat img = cv::cvarrToMat( timg, true );
	cv::Mat tmpl = cv::cvarrToMat( ttmpl, true );
	cvReleaseImage (&timg );
	cvReleaseImage (&ttmpl );
	return findItems( img, tmpl, meth );

}
QList<QRect> ImgProc::findItems( cv::Mat &img, cv::Mat &tmpl, int meth )
{
	cv::Mat result( cvSize(img.size().width, img.size().height), CV_32FC1 );
	
	QList<QRect> res;

	if ( meth > CV_TM_CCOEFF_NORMED )
	{
		vector<cv::KeyPoint> objectKeypoints;
		vector<cv::KeyPoint> sceneKeypoints;
		cv::Mat objectDescriptors;
		cv::Mat sceneDescriptors;

		//StarFeatureDetector detector;
		SimpleBlobDetector::Params p;
		p.maxArea = 50000;
		p.minArea = 10;
		p.filterByArea = true;
		p.filterByColor = false;
		p.filterByCircularity = false;
		p.filterByConvexity = false;
		p.filterByInertia = false;
		p.blobColor=255;
		SimpleBlobDetector detector(p);
		detector.detect(tmpl, objectKeypoints);
		detector.detect(img, sceneKeypoints);

		

		for (int i=0; i< sceneKeypoints.size(); ++i)
		{
			KeyPoint kp = sceneKeypoints.at(i);
			res.append(QRect(kp.pt.x*2, kp.pt.y*2, kp.size*2, kp.size*2));
		}
	}
	else if ( meth <= CV_TM_CCOEFF_NORMED && meth >= CV_TM_SQDIFF )
	{
		//cv::normalize( tmpl, tmpl, 0, 1, NORM_MINMAX, -1, Mat() );


		//imshow( "Image", img );
		//imshow( "Template", tmpl );
		
		matchTemplate( img, tmpl, result, meth );
		
		//cv::normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

		/// Localizing the best match with minMaxLoc
		  double minVal; 
		  double maxVal; 
		  Point minLoc; 
		  Point maxLoc;
		  Point matchLoc;

		  minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
		  //threshold( result, result, maxVal * 0.8, 255, CV_THRESH_BINARY );

		  cv::Mat greyRes;
		  /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
		  if( meth  == CV_TM_SQDIFF || meth == CV_TM_SQDIFF_NORMED )
		  { 
			  threshold( result, result, (maxVal-minVal)/4 +minVal, 255, CV_THRESH_BINARY_INV );
			  matchLoc = minLoc; 
		  }
		  else
		  { 
			  threshold( result, result, maxVal - (maxVal-minVal)/4 , 255, CV_THRESH_BINARY );
			  matchLoc = maxLoc; 
		  }

		  //std::vector<std::vector<cv::Point> > contours;
		  cv::vector<cv::vector<cv::Point> > contours;

		  

		  //result.convertTo( greyRes, CV_8UC1 );
		//cvtColor(result, greyRes, COLOR_BGR2GRAY);
	    result.convertTo(greyRes, CV_8UC1, 255, 0); 
		imshow( "grey", greyRes );
		cv::findContours(greyRes, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		return res;
		int foundBlobs=0;
		int areaTempl = tmpl.size().area();
		for( unsigned int i = 0; i < contours.size(); i++ )
		{ 
			if( contours[i].size() < 3  )
				continue;

			Mat mat = Mat(contours[i]);
			
			CvRect br = minAreaRect( mat ).boundingRect();
			int foundObjArea = br.width * br.height;

			if ( foundObjArea < areaTempl*16 ) // 4 times as big? skip
			{
				res.append( QRect( 0,0, br.width, br.height ) );
			}
		}


		  //rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
		  //rectangle( result, matchLoc, Point( matchLoc.x + tmpl.cols , matchLoc.y + tmpl.rows ), Scalar::all(255), 2, 8, 0 );

		  /*cv::Rect roi( cv::Point( 0, 0 ), result.size() );
		  Mat roim = show_result( roi );
		  result.copyTo( roim );

		  roi = cv::Rect( cv::Point( result.size().width-1, 0 ), img.size() );
		  roim = show_result( roi );
		  Mat img32f = Mat( img.size() , CV_32FC1 );
		  img.convertTo( img32f, CV_32FC1);		  
		  img32f.copyTo( roim );
		  */



	    //imshow( "Orig", img );
		//imshow( "match", result );
	}

	return res;



/*	
	char* imgdata = (char*)g_gray->imageData;
    int index = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{			
            char pix =  (char) qGreen(img.pixel(x, y));
			imgdata[index++] = pix;
		}
        index += index % g_gray->align;
	}		
	//cvShowImage("test", g_gray);
	//cvSmooth(g_gray, g_gray, CV_GAUSSIAN, 7, 7); 	
	//cvSobel(g_gray, g_out, 1, 0,3); 
	//cvLaplace(g_gray, g_gray, 3);
    cvThreshold( g_gray, g_gray, th, 255, CV_THRESH_BINARY );
	
	cvShowImage("test1", g_gray);
	CvSeq* contours = 0;
	CvMemStorage* storage = cvCreateMemStorage(0);

	cvFindContours( g_gray, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	if ( contours )
	{
		for (CvSeq*cont = contours; cont != 0; cont = cont->h_next)
		{
			CvContour  *contour = (CvContour *)cont;
			if (  ( contour->rect.height >= minsize && contour->rect.height <= maxsize )
				&&( contour->rect.width >= minsize && contour->rect.width <= maxsize ) )
			{
				if (display)
				{
					QPainter p(&img);
					QPen pen(Qt::SolidLine);
					pen.setWidth(1);
					pen.setColor(QColor("blue"));
					p.setPen( pen );
					p.drawRect(QRect(contour->rect.x, contour->rect.y, contour->rect.width, contour->rect.height ));
				}
				ancaes.append(QRect(contour->rect.x, contour->rect.y, contour->rect.width, contour->rect.height));
			}
		}
	}
	cvReleaseImage (&g_gray );
	*/


}

QList<QPolygon> ImgProc::getHuMoments( QImage & imgIn, int minsize, int maxsize, double* hu )
{
	QList<QPolygon> ellipses;
	try{

	QImage  img = ImageCache::instance().getImage(imgIn, QSize(imgIn.width()/4, imgIn.height()/4));
	QImage norm = img;
	normalize(norm);
	IplImage * orig = createGreyFromQImage(norm);
	diffXY( img, 5 ); // 1st derivation for better extraction of contours	
	IplImage * g_gray = createGreyFromQImage(img);

	cv::Mat src_gray = cv::cvarrToMat( g_gray, true );
	cvReleaseImage (&g_gray );

	cv::Mat src_orig = cv::cvarrToMat( orig, true );
	cvReleaseImage (&orig );

	
	minsize /=4;
	maxsize /=4;


	Mat threshold_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detect edges using Threshold
  threshold( src_gray, threshold_output, 1, 255, THRESH_BINARY );
  /// Find contours
  findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Find the rotated rectangles and ellipses for each contour
  vector<RotatedRect> minRect( contours.size() );
  vector<RotatedRect> minEllipse( contours.size() );

  double humean[7]={0.0};
  int cnt=0;
  for( int i = 0; i < contours.size(); i++ )
	{ 
		if( contours[i].size() < 5 )
			continue;

		Mat mat = Mat(contours[i]);
		
		RotatedRect rr = minAreaRect( mat );
		if (( rr.size.width >= minsize && rr.size.width <= maxsize  )
			&&
			( rr.size.height >= minsize && rr.size.height <= maxsize  ) )
		{
			cv::Rect br = cv::boundingRect( mat );
			ellipses.append(QRect(br.x*4,br.y*4,br.width*4,br.height*4));
			Mat cell = src_orig(br);
			//cv::norm
			//imshow("cell", cell);
			Moments m =  moments(cell);			
			HuMoments(m, humean );
			++cnt;
		}
	}
  for (int i=0; i<7; ++i)
	hu[i] = humean[i] / cnt;
}
catch (std::exception ex)
{
	
}
	return ellipses;
}
QList<QPolygon> ImgProc::findEllipses( QImage & imgIn, int minsize, int maxsize )
{
	return findEllipses1( imgIn, minsize, maxsize );
}

QList<QPolygon> ImgProc::findEllipses1( QImage & imgIn, int minsize, int maxsize )
{
	QImage  img = imgIn;//.scaled( imgIn.width()/4, imgIn.height()/4 );
	QList<QPolygon> ellipses;

	int width = img.width();
	int height = img.height();
	CvSize Size;
	Size.height = height;
	Size.width = width;


	diffXY( img, 5 ); // 1st derivation for better extraction of contours

	IplImage * g_gray = createGreyFromQImage(img);
	//IplImage* scaled1 = cvCreateImage( cvSize(tmpgrey->width/2,tmpgrey->height/2), tmpgrey->depth, tmpgrey->nChannels );
	//cvPyrDown(tmpgrey,scaled1);
	//IplImage* g_gray = cvCreateImage( cvSize(scaled1->width/2,scaled1->height/2), scaled1->depth, scaled1->nChannels );
	//cvPyrDown(scaled1,g_gray);
	//cvReleaseImage (&scaled1 );
	//cvReleaseImage (&tmpgrey );
	//cvShowImage("diff", g_gray);		
	cvThreshold( g_gray, g_gray, 1, 255,  CV_THRESH_BINARY );
	cvErode(g_gray, g_gray);
	//cvShowImage("grey_th", g_gray);
	
	CvSeq* contours = 0;
	CvMemStorage* storage = cvCreateMemStorage(0);

	cvFindContours( g_gray, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	if ( contours )
	{
		for (CvSeq*cont = contours; cont != 0; cont = cont->h_next)
		{
							
			CvContour  *contour = (CvContour *)cont;
			if(contour->total >= 6)
			{
				CvBox2D el = cvFitEllipse2(contour);
				if (  ( el.size.height >= minsize && el.size.height <= maxsize ) &&( el.size.width >= minsize && el.size.width <= maxsize ) )
				{
					ellipses.append(	
						QRect( QPoint( (el.center.x  - el.size.width/2), (el.center.y  -  el.size.height/2) ), 
						QSize( el.size.width, el.size.height ) ) 
					);
				}
			}			
		}
	}
	
	cvReleaseImage (&g_gray );	
	cvReleaseMemStorage( &storage);

	return ellipses;

}

double ImgProc::findPeaks( QImage & img, int diamMin, int diamMax, int levelOf5, QList<QRect> * peaks )
{
	
	//1. Prepare cv image
	IplImage *orig = createGreyFromQImage(img);
	//cvShowImage("or", orig);

	//2. Resize Image, use only 1/2 of size. This implies, the smallest peak could be 2x2
	IplImage* workimg = cvCreateImage( cvSize(orig->width/2,orig->height/2), orig->depth, orig->nChannels );
	cvPyrDown(orig,workimg);
	diamMin /= 2;
	diamMax /= 2;	
	//cvShowImage("pd", workimg);
	
	//3. reduce greyscale to 5 per pixel. This is very inteligeng cv-function, which scales in very sensible way.
	//TODO: dont do it for nDNA, as there are no scales, nearly binary image idicating posotove areas
	//      nake normalizing configurable
	//cvNormalize(workimg,workimg,0,5,CV_MINMAX);
	//3.1. by default take 2 last layers fromscale. Configure by levelOf5
	int th = (255/5) * levelOf5;
	cvThreshold( workimg, workimg, th, 255,  CV_THRESH_BINARY );
	//cvShowImage("nrm1", workimg);

	//4. Prepare cv helper-data
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    
	//5. find contours of peaks
    cvFindContours( workimg, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	//6. Select those, fitting size
	double cellsMean=0;
	double cellsAreaMean=0;
	double cellsAreaVar=0;
	QList <double> cellsAreas;

	CvScalar s = cvAvg( workimg );
	double imgAvg = s.val[0];

	int cnt=0;
    if ( contours )
    {
	    for (CvSeq*cont = contours; cont != 0; cont = cont->h_next)
	    {
		    CvContour  *contour = (CvContour *)cont;
		    if ( //contours in size of cell //TODO: use variables
                
                ( ( contour->rect.height >= diamMin && contour->rect.height <= diamMax )
			      &&( contour->rect.width >= diamMin && contour->rect.width <= diamMax ) )
                  ||
                 ( ( contour->rect.height >= diamMin && contour->rect.height <= diamMax )
			      &&( contour->rect.width >= diamMin * 1.5 && contour->rect.width <= diamMax * 1.5 ) )
                  ||
                 ( ( contour->rect.height >= diamMin * 1.5 && contour->rect.height <= diamMax * 1.5 )
			      &&( contour->rect.width >= diamMin && contour->rect.width <= diamMax ) )
                  
               )
		    {
				cvSetImageROI( workimg, contour->rect );
				CvScalar s = cvAvg( workimg );
				cvResetImageROI( workimg );
				cellsMean += s.val[0];
				cellsAreaMean += (contour->rect.height * contour->rect.width);
				// Rescaling to original size from pyrDown-size
				if (peaks)
					peaks->append(QRect(contour->rect.x*2, contour->rect.y*2, contour->rect.width*2, contour->rect.height*2));
				cellsAreas.append( (contour->rect.height * contour->rect.width) );
				++cnt;
		    }
	    }
    }

    cvReleaseMemStorage( &storage );    
	
	cvReleaseImage (&workimg );
	cvReleaseImage (&orig );	

	return cnt;
}


bool ImgProc::merge( QImage &imgout, const QImage &imgAdd )
{
    if ( !imgout.isNull() && (imgAdd.size() != imgout.size()) ){
       return false;
    }

	if ( imgout.isNull() )
	{
		imgout = imgAdd;
	}
	else
	{
		quint8 lc1=0, lc2=0;
		for ( int y=0; y < imgAdd.height(); ++y )
		{
			for ( int x=0; x <imgAdd.width()-1; ++x )
			{
				quint8 c1 = imgAdd.pixel(x,y) >> 8;
				quint8 c2 = imgout.pixel(x,y) >> 8;
				int diff1 = abs(c1 - lc1);
				int diff2 = abs(c2 - lc2);
				if ( diff1 > diff2 )
				{
					imgout.setPixel(x,y,(unsigned long)c1<<8 );
				}
				lc1=c1;
				lc2=c2;
			}
		}
	}
    return true;
}


	
//TODO: optimize, don't use pixel() use data or scanline for pixel, setPixel access!!
qint32 ImgProc::calculateFocusValue( QImage & img, int alg, QRect tile, bool debugMode )
{
	int afValue=0;
	

	static int i=0;

	if( img.width() < 10 || img.height()<10 )
		return 0;

	if (tile.isEmpty())
	{
		tile = QRect(QPoint(0,0),img.size());
	}

	int focusHeight = tile.height();
	int focusWidth  = tile.width();
	int focusPosX = tile.x();
	int focusPosY = tile.y();	
    	
    static int lineHistogram[256]={0};
    QVector<quint8> imgLine(img.width());

	int imgWidth =  img.width();	
	int line=tile.y(); 
		
    

    int treshHold = 1;
	// Algorithm Type:1
	// ================
	// Works line-wise
	// 1. Calculates pixel mean-value for line
	// 2. Calculates mean-values for 4 equal line-sectors
	// 3. Comparing difference between means of 2 groups of 3 consecutive pixels
	const int analyzeSections = 4;
    if ( alg == 1 )
    {    
		int q=focusWidth/analyzeSections;
		int focusPosXTo = focusPosX + focusWidth;
		int focusPosYTo = focusPosY + focusHeight;
		PDImageInfo imginf;
		getImageInfo( img, imginf );
        for (int y =focusPosY; y < focusPosYTo; y++)
        {
			int mid[analyzeSections]={0};
			for (int i=0; i< analyzeSections;++i) mid[i]=0;
			double linemid = 0.0;
			quint8 pc=0;
            for (int x =focusPosX; x < focusPosXTo; ++x)
            {
                quint8 c = img.pixel(x,y) >> 8;  
				if ( c == 255)
					c=pc; //discard overexposed
				mid[(x-focusPosX)/q]+=c;
				pc=c;
				linemid += c;

            }
			linemid  /= (focusPosXTo - focusPosX);

			//Use only for adaptive mean-calculation. not as filter!!!
			for (int i=0; i<4; ++i)
			{
				mid[i] /= focusWidth / analyzeSections;
			}

			quint8 a1=0,a2=0,a3=0,a4=0,a5=0,b1=0,b2=0,b3=0,b4=0,b5=0;

			QRgb * pix = (QRgb *)img.scanLine(y);
			pix += focusPosX;
			int edges=0;
            for (int x =focusPosX; x < focusPosXTo; ++x)
            {	
				int midIdx = (x-focusPosX)/q;
				

				a1=a2;
				a2=a3;
				a3=b1;
				b1=b2;
				b2=b3;				
				b3=b4;
				b4=b5;
				b5 = qGreen( *(pix++) );

				if ( b5==255  ) // overexposed, don't count
				{
					//2.0.5 b3=b2
					b5=a5;
				}
				b5 = (b5>=mid[midIdx]? b5-mid[midIdx] : 0);
				
				
				
				if ( imginf._brightness < 60 )
				{
					int ma = (a1+a2+a3+a4+a5)/5;
					int mb = (b1+b2+b3+b4+b5)/5;
					double diff = ma>mb ? ma-mb : mb-ma;

					if (  diff > 60 ) //On dark images such a contrast indicates an inhomogenious, bright region
						return -1;

					if (  diff > 30 ) //On dark images such a contrast indicates an inhomogenious, bright region
						break;

				afValue += diff*(diff/2);
                }
				else //Emulating "error" in 2.0.5 for bright images. The rounding error caused discarding AF on bg (low contrast)
				{
					int diff = qAbs( (a1+a2)/2 - (a3+b4)/2 );

					if ( diff < imginf._brightness/2 ) //Ignore too bright regions.
					afValue += diff*(diff/2);
				}												
            }
            }
        }

	//Same as 1 but considering overexposed
	if ( alg == 7 )
    {    
		int q=focusWidth/4;
		int focusPosXTo = focusPosX + focusWidth;
		int focusPosYTo = focusPosY + focusHeight;
		PDImageInfo imginf;
		getImageInfo( img, imginf );
        for (int y =focusPosY; y < focusPosYTo; y++)
        {
			int mid[4]={0,0,0,0};
			double linemid = 0.0;
            for (int x =focusPosX; x < focusPosXTo; ++x)
            {
                quint8 c = img.pixel(x,y) >> 8;  
				mid[(x-focusPosX)/q]+=c;
				linemid += c;

            }
			linemid  /= (focusPosXTo - focusPosX);

			//Use only for adaptive mean-calculation. not as filter!!!
			for (int i=0; i<4; ++i)
			{
				mid[i] /= focusWidth / 4;
			}

			quint8 a1=0,a2=0,a3=0,b1=0,b2=0,b3=0;

			QRgb * pix = (QRgb *)img.scanLine(y);
			pix += focusPosX;
			int edges=0;
            for (int x =focusPosX; x < focusPosXTo; ++x)
            {			
				int midIdx = (x-focusPosX)/q;

				a1=a2;
				a2=a3;
				a3=b1;
				b1=b2;
				b2=b3;				
				b3 = qGreen( *(pix++) );

				b3 = (b3>=mid[midIdx]? b3-mid[midIdx] : 0);
				
                int ma = (a1+a2+a3)/3;
                int mb = (b1+b2+b3)/3;
				
				if ( imginf._brightness < 40 )
				{
					double diff = ma>mb ? ma-mb : mb-ma;
					afValue += diff*(diff/2);
				}
				else //Emulating "error" in 2.0.5 for bright images. The rounding error caused discarding AF on bg (low contrast)
				{
					int diff = qAbs( (a1+a2)/2 - (a3+b1)/2 );
					afValue += diff*(diff/2);
				}												
            }
        }
    }

	if ( alg == 2 )
    {    
		int q=focusWidth/4;
		int focusPosXTo = focusPosX + focusWidth;
		int focusPosYTo = focusPosY + focusHeight;
		PDImageInfo imginf;
		getImageInfo( img, imginf );
		if ( imginf._histogramWidth < 50 )
		{
			afValue = 0;
		}
		else
		{
			for (int y =focusPosY; y < focusPosYTo; y++)
			{
				int mid[4]={0,0,0,0};
				
				for (int x =focusPosX; x < focusPosXTo; ++x)
				{
					quint8 c = img.pixel(x,y) >> 8;  
					mid[(x-focusPosX)/q]+=c;
				}
				for (int i=0; i<4; ++i) mid[i] /= focusWidth / 4;	
				quint8 a1=0,a2=0,a3=0,b1=0,b2=0,b3=0;

				for (int x =focusPosX; x < focusPosXTo; ++x)
				{	
					a1=a2;
					a2=a3;
					a3=b1;
					b1=b2;
					b2=b3;				
					b3 = img.pixel(x,y) >> 8;
					
       				if ( b3==255  ) // overexposed, don't count
					{
						b3=0;
					}
					b3 = (b3>=mid[(x-focusPosX)/q]? b3-mid[(x-focusPosX)/q] : 0);
					int ma = (a1+a2+a3)/3;
					int mb = (b1+b2+b3)/3;
					int diff = ma>mb ? ma-mb : mb-ma;
					if ( diff > 5 )
					{
						afValue += 1;
					}
				}
			}
        }
    }
	else if ( alg == 3 ) // histogram based
    { 		
		PDImageInfo imginf;
		getImageInfo( img, imginf );
		if ( imginf._histogramWidth < 50 )
		{
			afValue = 0;
		}
		else
		{
			afValue = imginf._histogramWidth * 100; //*100 because of old threshholds user in fsms
		}
	}

    else if ( alg == 4 )
    { 		
        imgWidth -= 8; // analysing 8 neighbouring pix array
        do{
            
            memset( lineHistogram, 0, sizeof(lineHistogram) );
			for ( int x=tile.x(); x < tile.x()+tile.width(); ++x )
            {            
		        quint8 c1 = img.pixel(x,line) >> 8;
                lineHistogram[c1]++;
                imgLine[ x ] = c1;
            }	
            bool spinUp=false;
            quint8 minIntensity=255;
            quint8 maxIntensity=0;
            
            for (int i=0; i <= 255; ++i)
            {
                if ( lineHistogram[i] > treshHold )
                {
                    if ( minIntensity == 255 ){
                        minIntensity = i;                    
                    }
                    maxIntensity = i;
                }            
            }
            quint8 hysteresisValHigh= (maxIntensity - minIntensity) / 3*2 + minIntensity;
            quint8 hysteresisValLow= (maxIntensity - minIntensity) / 3 + minIntensity;
            float levelLowMid=1;
            float levelHighMid=1;
            //qDebug() << hysteresisVal<< maxIntensity << minIntensity<<" Diff:"<< (maxIntensity - minIntensity);
            int histWidth = maxIntensity - minIntensity;
            
            if ( histWidth > 2 )
            {
                int lineSpins=0;
                unsigned long col;
				bool firstSpin = true;
                for ( int x=0; x < imgWidth; ++x )
                {
		            quint8 c1 = imgLine[x];
                    quint8 c2 = imgLine[x+1];
                    quint8 c3 = imgLine[x+2];
                    quint8 c4 = imgLine[x+3];
                    quint8 c5 = imgLine[x+4];
                    quint8 c6 = imgLine[x+5];
                    quint8 c7 = imgLine[x+6];
                    quint8 c8 = imgLine[x+7];
                    if ( !spinUp && 
                         ( c1>hysteresisValHigh && c2>hysteresisValHigh && c3>hysteresisValHigh && c4>hysteresisValHigh && c5>hysteresisValHigh
                         && c5>hysteresisValHigh && c6>hysteresisValHigh && c7>hysteresisValHigh
                         )
                       )
                    {
                        spinUp=true;
						if ( !firstSpin )
						{
							++lineSpins;						
							if ( debugMode )
							{
								col=0xFFFF0000;
								img.setPixel( x, line, col );
								img.setPixel( x+1, line, col );
								img.setPixel( x+2, line, col );
								img.setPixel( x+3, line, col );
							}
							
						}
						firstSpin = false;
                        x += 7;
                        levelHighMid = (c1+c2+c3+c4+c5+c6+c7+c8)/8;
                    }
                    else if ( spinUp && 
                         ( c1<hysteresisValLow && c2<hysteresisValLow && c3<hysteresisValLow && c4<hysteresisValLow && c5<hysteresisValLow
                         && c6<hysteresisValLow && c7<hysteresisValLow && c8<hysteresisValLow)
                       )
                    {
                        spinUp=false;
                        ++lineSpins;
						if ( debugMode )
						{
							col=0xFF0000FF;
							img.setPixel( x, line, col );
							img.setPixel( x+1, line, col );
							img.setPixel( x+2, line, col );
							img.setPixel( x+3, line, col );
						}
                        x += 7;
                        levelLowMid = (c1+c2+c3+c4+c5+c6+c7+c8)/8;
                    }                    
                }
				if ( levelLowMid>0 && (lineSpins > 4) && (lineSpins < 15) )
                {          
					//if (levelLowMid>0)
					{
						afValue += histWidth * (levelHighMid / levelLowMid);
					}
					//afValue += lineSpins;
                }
            }
            line++;
        }while ( line < (tile.y() + tile.height()) );
	    
    }
	if ( alg == 5 ) //former alg 1. preserved for ANCAs
    {    
		int q=focusWidth/4;
		int focusPosXTo = focusPosX + focusWidth;
		int focusPosYTo = focusPosY + focusHeight;
		PDImageInfo imginf;
		getImageInfo( img, imginf );

        for (int y =focusPosY; y < focusPosYTo; y++)
        {
			int mid[4]={0,0,0,0};
			
            for (int x =focusPosX; x < focusPosXTo; ++x)
            {
                quint8 c = img.pixel(x,y) >> 8;  
				mid[(x-focusPosX)/q]+=c;
            }
			for (int i=0; i<4; ++i) mid[i] /= focusWidth / 4;	
			quint8 a1=0,a2=0,a3=0,b1=0,b2=0,b3=0;

            for (int x =focusPosX; x < focusPosXTo; ++x)
            {	
				a1=a2;
				a2=a3;
				a3=b1;
				b1=b2;
				b2=b3;				
				b3 = img.pixel(x,y) >> 8;

				b3 = (b3>=mid[(x-focusPosX)/q]? b3-mid[(x-focusPosX)/q] : 0);
                int ma = (a1+a2+a3)/3;
                int mb = (b1+b2+b3)/3;
                int diff = ma>mb ? ma-mb : mb-ma;
                if ( diff > 1 )
                {
                    afValue += diff;
                }
            }
        }

		if ( imginf._histogramWidth > 100 )
			afValue += imginf._histogramWidth*5;
    }

	if ( alg == 6 ) //2nd derivation
    {    
		int q=focusWidth/4;
		int focusPosXTo = focusPosX + focusWidth;
		int focusPosYTo = focusPosY + focusHeight;

        for (int y =focusPosY; y < focusPosYTo; y++)
        {
			int mid[4]={0,0,0,0};
			double linemid = 0.0;
			quint8 pc=0;
            for (int x =focusPosX; x < focusPosXTo; ++x)
            {
                quint8 c = img.pixel(x,y) >> 8;  
				if ( c == 255)
					c=pc; //discard overexposed
				mid[(x-focusPosX)/q]+=c;
				pc=c;
				linemid += c;

            }
			linemid  /= (focusPosXTo - focusPosX);

			//Use only for adaptive mean-calculation. not as filter!!!
			for (int i=0; i<4; ++i)
			{
				mid[i] /= focusWidth / 4;
			}

			quint8 a1=0,a2=0,a3=0,b1=0,b2=0,b3=0;

			QRgb * pix = (QRgb *)img.scanLine(y);
			pix += focusPosX;
			int edges=0;
			double linediff=0;
			int lineOverexposedCnt = 0;
			for (int x =focusPosX; x < focusPosXTo; ++x)
            {			
				int midIdx = (x-focusPosX)/q;

				a1=a2;
				a2=a3;
				a3= qGreen( *(pix++) );

				if ( a3 == 255 ) //discard overexposed
				{
					a3=a1;
					//lineOverexposedCnt++;
				}

				/*if ( lineOverexposedCnt > 30 )
				{
					linediff=0;
					break;
				}*/

				if (( a1<a2 && a2 < a3) || ( a1>a2 && a2 > a3))
				{
					double pdiff= qAbs( a3-a2 ) / qAbs( a2-a1 );
					//if (pdiff > 3)
						linediff += (pdiff *pdiff);// * qAbs( a3-a1 );
				}
            }
			afValue += qSqrt( linediff ) * 10; //*10 only for adjusting scale
        }
    }

	return afValue;
}

void ImgProc::diffX( QImage & img, int th )
{
	int w=img.width();
	int h=img.height();
	QImage diff = img;

    for (int y =0; y < h; y++)
    {			
        for (int x =1; x < w; ++x)
        {
            int c = img.pixel(x,y) >> 8;
			int c1 = img.pixel(x-1,y) >> 8;  
			quint8 cd = (quint8) qAbs( c1 - c );
			if (cd < th)
			{
				cd=0;
				diff.setPixel( x, y, 0 );
			}
			else
				diff.setPixel( x, y, cd << 8 );
        }
	}

	img = diff;
}

void ImgProc::diffY( QImage & img, int th )
{
	int w=img.width();
	int h=img.height();
	QImage diff = img;

    for (int y =1; y < h; y++)
    {			
        for (int x =0; x < w; ++x)
        {
            int c = img.pixel(x,y) >> 8;
			int c1 = img.pixel(x,y-1) >> 8;  
			quint8 cd = (quint8) qAbs( c1 - c );
			if (cd < th)
			{
				cd=0;
				diff.setPixel( x, y, 0 );
			}
			else
				diff.setPixel( x, y, cd << 8 );
        }
	}

	img = diff;
}

void ImgProc::diffXY( QImage & img, int th )
{
	int w=img.width();
	int h=img.height();
	QImage diff = img;

    for (int y =1; y < h; y++)
    {			
        for (int x =1; x < w; ++x)
        {
            int cy = img.pixel(x,y) >> 8;
			int cy1 = img.pixel(x,y-1) >> 8;  

			int cx = img.pixel(x,y) >> 8;
			int cx1 = img.pixel(x-1,y) >> 8;  


			quint8 cd = (quint8) (qAbs( cx1 - cx ) + qAbs( cy1 - cy ))/* /2 */;
			if (cd < th)
			{
				cd=0;
				diff.setPixel( x, y, 0 );
			}
			else
				diff.setPixel( x, y, cd << 8 );
        }
	}

	img = diff;
}


void ImgProc::overlayImageInfo( QImage &cimg, const QPoint & pos  )
{
    PDImageInfo imginf;
    getImageInfo( cimg, imginf );
    QPainter p( &cimg );
    p.setPen( QPen(QBrush(QColor(Qt::white)),2) );
    p.drawRect( QRect(pos,QSize(512,200)) );
    p.setPen( QPen(QBrush(QColor(Qt::blue)),2) );
    double f = 0.25;
    for (int i=0; i<512; i+=2)
    {
        unsigned long hv = imginf._histogram[i/2] * f;
		if ( i == 510 )
		{
			p.setPen( QPen(QBrush(QColor(Qt::gray)),5) );
		}
        p.drawLine(pos.x()+i, pos.y()+200, pos.x()+i, pos.y()+ (200 - ( hv <= 200 ? hv : 200 )) );
    }
    p.setPen( QPen(QBrush(QColor(Qt::red)),5) );
    
    p.setPen( QPen(QBrush(QColor(Qt::white)),2) );
    QFont fnt(p.font());
    int imgwidth = cimg.width();

    fnt.setPixelSize(imgwidth>1900?29:imgwidth>1000?20:16);
    p.setFont(fnt);
    p.drawText(QPoint(pos.x()+260*2,60), QString( "Min:%1" ).arg(imginf._histogramMin) );
    p.drawText(QPoint(pos.x()+260*2,90), QString( "Max:%1" ).arg(imginf._histogramMax) );
	p.drawText(QPoint(pos.x()+260*2,120), QString( "Brtn:%1" ).arg(imginf._brightness) );
	p.drawText(QPoint(pos.x()+260*2,150), QString( "Mean:%1" ).arg(imginf._meanPixValue) );
}

void ImgProc::reduceBackgroundNoise( QImage &img, int bgvalue )
{
	QImage out = img;
	for ( int y=0; y < img.height()-1; ++y )
	{
		for ( int x=0; x <img.width()-1; ++x )
		{
			quint8 c1 = img.pixel(x,y) >> 8;

			if ( c1 < bgvalue )
			{
				int sub = (bgvalue-c1) ;
				c1 -= sub <= c1 ? sub : c1;
				out.setPixel(x,y,c1 << 8 );
			}
		}
	}
	img = out;
}

void ImgProc::removeBackground( QImage &img, int bgvalue )
{
	QImage out = img;
	for ( int y=0; y < img.height()-1; ++y )
	{
		for ( int x=0; x <img.width()-1; ++x )
		{
			quint8 c1 = img.pixel(x,y) >> 8;

			if ( c1 < bgvalue )
			{
				out.setPixel(x,y,0 );
			}
		}
	}
	img = out;
}

//s. http://www.aishack.in/2010/07/drawing-histograms-in-opencv/
IplImage* DrawHistogram(IplImage*img, float scaleX=1, float scaleY=1)
{
	int numBins = 256;
    float range[] = {0, 255};
    float *ranges[] = { range };
 
    CvHistogram *hist = cvCreateHist(1, &numBins, CV_HIST_ARRAY, ranges, 1);
    cvClearHist(hist);

	cvCalcHist ( &img , hist , 0, NULL );
    float histMax = 0;
    cvGetMinMaxHistValue(hist, 0, &histMax, 0, 0);

    IplImage* imgHist = cvCreateImage(cvSize(256*scaleX, 64*scaleY), 8 ,1);
    cvZero(imgHist);

    for(int i=0;i<255;i++)
    {
        float histValue = cvQueryHistValue_1D(hist, i);
        float nextValue = cvQueryHistValue_1D(hist, i+1);
 
        CvPoint pt1 = cvPoint(i*scaleX, 64*scaleY);
        CvPoint pt2 = cvPoint(i*scaleX+scaleX, 64*scaleY);
        CvPoint pt3 = cvPoint(i*scaleX+scaleX, (64-nextValue*64/histMax)*scaleY);
        CvPoint pt4 = cvPoint(i*scaleX, (64-histValue*64/histMax)*scaleY);
 
        int numPts = 5;
        CvPoint pts[] = {pt1, pt2, pt3, pt4, pt1};
 
        cvFillConvexPoly(imgHist, pts, numPts, cvScalar(255));
    }

    return imgHist;
}

//TODO return Mat
IplImage * createGreyFromQImage(QImage img, int ch)
{
	int width = img.width();
	int height = img.height();
	CvSize Size;
	Size.height = height;
	Size.width = width;
	IplImage *grayImg = cvCreateImage(Size, IPL_DEPTH_8U, 1);


	char* imgdata = (char*)grayImg->imageData;
	int index = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{			
			char pix =  (char) qGreen(img.pixel(x, y));
			imgdata[index++] = pix;
		}
		index += index % grayImg->align;
	}
	//cvNamedWindow( "orig", CV_WINDOW_AUTOSIZE );
	//cvShowImage("orig", grayImg);
	//cvShowImage("hist1", DrawHistogram( grayImg ));
	//cvEqualizeHist (grayImg , grayImg );
	//cvNamedWindow( "eqhist", CV_WINDOW_AUTOSIZE );
	//cvShowImage("eqhist", grayImg);
	

	//cvShowImage("hist2", DrawHistogram( grayImg ));

	return grayImg;


}

QImage IplImage2QImage(IplImage *iplImg)
{
    int h = iplImg->height;
    int w = iplImg->width;
    int channels = iplImg->nChannels;
    QImage qimg(w, h, QImage::Format_ARGB32);
    char *data = iplImg->imageData;

    for (int y = 0; y < h; y++, data += iplImg->widthStep)
    {
    for (int x = 0; x < w; x++)
    {
    char r, g, b, a = 0;
    if (channels == 1)
    {
    r = data[x * channels];
    g = data[x * channels];
    b = data[x * channels];
    }
    else if (channels == 3 || channels == 4)
    {
    r = data[x * channels + 2];
    g = data[x * channels + 1];
    b = data[x * channels];
    }

    if (channels == 4)
    {
    a = data[x * channels + 3];
    qimg.setPixel(x, y, qRgba(r, g, b, a));
    }
    else
    {
    qimg.setPixel(x, y, qRgb(r, g, b));
    }
    }
    }
    return qimg;

}

#endif