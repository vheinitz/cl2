/*V. Heinitz, 2012-04-05
Very experimental code with many copy&pastes and unused functions.
Among other things implements:
-calculation algorithms for focus-evaluation
-Simple pre-classification 
*/

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



QImage PDImageProcessing::analyseStripe(QImage qi)
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




Mat PDImageProcessing::toMat( QImage src, ChannelMode in, ChannelMode out )
{
	//TODO evaluate in out settings
	cv::Mat tmp(src.height(),src.width(),CV_8UC4,(uchar*)src.bits(),src.bytesPerLine());
    cv::Mat result;
    cvtColor(tmp, result,CV_BGRA2RGB);
	
	Mat channels[3];
	split(result,channels); 
    return channels[1];	
}

QImage PDImageProcessing::toQImage( const cv::Mat &src )
{ 
	return IplImage2QImage( new IplImage(src) );
}

void PDImageProcessing::rectify(QImage & img, QList<QPoint> corners, QRectF * br)
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

void PDImageProcessing::storeImageTask(QString path, QImage img, int beautify, TImageMap bestImages, QObject *callback)
{    
	StoreImageThread *tmpImgTask = new StoreImageThread(path, img, beautify, bestImages) ;
	if( callback )
		QObject::connect( tmpImgTask, SIGNAL(fileSaved(QString,QStringList)), callback, SLOT(processFileSaved(QString,QStringList)));

	StoreImageThread::_StoreImageTasks.append(tmpImgTask);
    QThreadPool::globalInstance()->start( tmpImgTask );
}

bool PDImageProcessing::storeImageTaskBusy(int & readypct)
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


void PDImageProcessing::reduceBgNoise(QImage & img)
{
    PDImageInfo iminf;
    getImageInfo(img, iminf);
    reduceBackgroundNoise(img, iminf._bgLevel1 );
}

void PDImageProcessing::sharpen(QImage & img)
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


void PDImageProcessing::beautifyImage(QString img)
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

bool PDImageProcessing::detectShift(QImage &img1, QImage &img2, int _tileSize, int &dx, int &dy )
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

double PDImageProcessing::compareHistogramm(QImage &imgo1, QImage &imgo2, int meth, QRect r)
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

bool PDImageProcessing::preClassifyNdna( QImage &img, int minDnasPercent, int &nDnaPct, int min, int max, int dots, int minCells, double maxExposure, int peakMin , int peakMax  )
{	

	double exp = img.text("exposure").toDouble();
	if ( exp > maxExposure )
	{
		return false;
	}
	
	QList<QPolygon> pol =  PDImageProcessing::findEllipses1( img, min, max );	
	QList<QRect> positions; //dummy	
	QList<QRect> cells;


	foreach( QPolygon p, pol )
	{
		QRect br = p.boundingRect();
		bool ignorePolygon=false;
		foreach( QRect cr, cells )
		{
			if( cr.intersects(br) ) // if rect on similar position exists
			{
				ignorePolygon=true;
				break;
			}
		}
		if( !ignorePolygon )
			cells.append( br );
	}

	if ( cells.size() < minCells )
	{
		return 0;
	}

	
	QList<QRect> peaks;
	PDImageProcessing::findPeaks( img, peakMin, peakMax, 3, &peaks );

	struct nDnaPosCandidate
	{
		QRect _rect;
		QList<QRect> _peaks;
		static QString mkKey( const QRect & r )
		{
			return QString("%1:%2 %3x%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
		}
	};

	QMap<QString,nDnaPosCandidate> posNdnas;
	
	if (  dots <= 0  )
		positions=cells;
	else
	{
		foreach( QRect cr, cells )
		{
			QString k = nDnaPosCandidate::mkKey(cr);
			foreach( QRect pr, peaks )
			{
				if ( cr.intersects( pr ) )
				{				
					posNdnas[ k ]._rect = cr;
					posNdnas[ k ]._peaks.append(pr);
				}
			}
		}
		
		foreach (QString k, posNdnas.keys() )
		{
			nDnaPosCandidate & nc = posNdnas[ k ];
			if( nc._peaks.size() >= dots )
			{
				positions.append( nc._rect );
			}
		}
	}

	double nDnaPerCellNumber = positions.size();
	nDnaPerCellNumber /= cells.size();
	nDnaPerCellNumber  *=100;
	nDnaPct = nDnaPerCellNumber;

	/*
	QPainter p(&img);
	QPen pen( QBrush(Qt::blue),5 );
	p.setPen( pen );
	foreach( QRect r, positions )
	{
		p.drawRect(r);
	}
	*/

	return nDnaPerCellNumber >= minDnasPercent;
}

/**/
bool PDImageProcessing::preClassifyAnca(  QImage &img,			//image for pre-classification
										   int minHistWidth,	//minimal histogram width for evaluating as positive
										   int maxBgFloor,		//maximal background level for evaluating as positive
										   int maxBgRatio,		//maximal background ratio
										   int minCellsPerLine, //minimal mean number of edges per line (discards big bright objects) 
										   int maxCellsPerLine, //maximal mean number of edges per line (discards noise, hot pixels)
										   PreClassificationResults & res,
										   double eptCF)
{
	PDImageInfo imginf;
	getImageInfo( img, imginf );
	res._histwidth = imginf._histogramWidth;
	res._bgFloor = imginf._bgLevel;
	res._bgRatio = imginf._bgRatio;
	bool result = true;

	bool exposureValid=false;	
	bool maxExposureValid=false;
	imginf._exposure = img.text("exposure").toDouble(&exposureValid);
	imginf._maxExposure = img.text("maxExposure").toDouble(&exposureValid);


	if ( res._histwidth < minHistWidth )
	{
		//TODO logging not thread-safe PD_TRACE( "Image has too low contrast for pre-classification" )
		res._histwidthRes = RFNok;
		result &= false;
	}
	else
	{
		res._histwidthRes = RFOk;
	}
	

	if ( imginf._bgLevel > maxBgFloor )
	{
		res._bgFloorRes = RFNok;
		//TODO logging not thread-safe PD_TRACE( "Image has too high exposure. May look like positive, but not." )
		result &= false;
	}
	else
	{
		res._bgFloorRes = RFOk;
	}

	if ( imginf._bgRatio > maxBgRatio )
	{
		res._bgRatioRes = RFNok;
		//TODO logging not thread-safe PD_TRACE( "Looks like not nucleolar - too high cell density, too less bg" )
		result &= false;
	}
	else
	{
		res._bgRatioRes = RFOk;
	}
	
	int focusHeight = img.height();
	int focusWidth  = img.width();
	int focusPosX = 0;
	int focusPosY = 0;	
	int focusPosXTo = focusWidth;
	int focusPosYTo = focusHeight;

	int q=focusWidth/4;
	int cellLines=0;


	QList<QPolygon>  nuclei;
	int nucleiIndex=0;
	nuclei = findEllipses1(img, 30, 100 );
	nucleiIndex = nuclei.size();

	res._midCellsPerLine = nucleiIndex*120 / 1920; //fitting to old-value order
	bool cplOK = (res._midCellsPerLine >= minCellsPerLine) && (res._midCellsPerLine <= maxCellsPerLine);


	res._midCellsPerLineRes = cplOK ? RFOk:RFNok;
	result &= cplOK;
	 
	res._titer = estimateTiter(imginf._brightness, imginf._exposure, imginf._overexposure1, eptCF );

	return result;
}

/**/
bool PDImageProcessing::preClassifyImage(  QImage &img,			//image for pre-classification
										   int minHistWidth,	//minimal histogram width for evaluating as positive
										   int maxBgFloor,		//maximal background level for evaluating as positive
										   int maxBgRatio,		//maximal background ratio
										   int minCellsPerLine, //minimal mean number of edges per line (discards big bright objects) 
										   int maxCellsPerLine, //maximal mean number of edges per line (discards noise, hot pixels)
										   PC_DCPSettings & nuclearSettings,
										   PreClassificationResults & res,
										   double eptCF)
{

	PDImageInfo imginf;
	getImageInfo( img, imginf );
	res._histwidth = imginf._histogramWidth;
	res._bgFloor = imginf._bgLevel;
	res._bgRatio = imginf._bgRatio;
	bool result = true;

	bool exposureValid=false;	
	bool maxExposureValid=false;
	imginf._exposure = img.text("exposure").toDouble(&exposureValid);
	imginf._maxExposure = img.text("maxExposure").toDouble(&exposureValid);


	if ( nuclearSettings._active )
	{		
		int nuclearIndex=0;
		//findHEp2Nucleus1( img, nuclearSettings._nuclei, nuclearIndex, nuclearSettings._min, nuclearSettings._max );
		int cytoplasmaIndex = 100 - nuclearIndex;
		int cells = nuclearSettings._nuclei.size();
		res._cytoplasmaIndex = cytoplasmaIndex;
		if ( cytoplasmaIndex >= nuclearSettings._maxCytoplasmaIndex )
		{			
			res._cytoplasmaIndexRes = RFNok;
			result &= false;
		}
		else
		{
			res._cytoplasmaIndexRes = RFOk;
		}
	}
	
	if ( res._histwidth < minHistWidth )
	{
		//TODO logging not thread-safe PD_TRACE( "Image has too low contrast for pre-classification" )
		res._histwidthRes = RFNok;
		result &= false;
	}
	else
	{
		res._histwidthRes = RFOk;
	}
	

	if ( imginf._bgLevel > maxBgFloor )
	{
		res._bgFloorRes = RFNok;
		//TODO logging not thread-safe PD_TRACE( "Image has too high exposure. May look like positive, but not." )
		result &= false;
	}
	else
	{
		res._bgFloorRes = RFOk;
	}

	if ( imginf._bgRatio > maxBgRatio )
	{
		res._bgRatioRes = RFNok;
		//TODO logging not thread-safe PD_TRACE( "Looks like not nucleolar - too high cell density, too less bg" )
		result &= false;
	}
	else
	{
		res._bgRatioRes = RFOk;
	}
	
	//Calculating cells per line
	QRect(QPoint(img.width()/4,img.height()/4), QSize(img.width()/2, img.height()/2 ) );
	int focusHeight = img.height()/2;
	int focusWidth  = img.width()/2;
	int focusPosX = img.width()/4;
	int focusPosY = img.height()/4;	
	int focusPosXTo = focusPosX + focusWidth;
	int focusPosYTo = focusPosY + focusHeight;
	int q=focusWidth/4;
	int cellLines=0;


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

		int cellsInLine=0;
		int cell=0;
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
			if ( diff > 3 && mb>ma)
			{
					cell=1; //on cell
			}
			else if ( cell && (diff > 1) && mb<ma)
			{
					if( ((b1+b2+b3)/3)<=imginf._bgLevel )
					{
						cell=0; //on bg (again)
						cellsInLine++;
						/*if( showDebugInfo )
						{
							img.setPixel( x, y, (0xFF0000FF | b3<<8)  );
						}*/
					}
			}
			
			/*if ( cell )
			{
				if( showDebugInfo )
				{
					img.setPixel( x, y, (0xFFFF0000 | b3<<8)  );
				}
			}*/
		}
		if ( cellsInLine > 0 )
		{
			cellLines++;
			res._midCellsPerLine +=cellsInLine;

		}
	}

	if ( cellLines>0 )
	{
		res._midCellsPerLine /= cellLines;
	}

	bool cplOK = (res._midCellsPerLine >= minCellsPerLine) && (res._midCellsPerLine <= maxCellsPerLine);
	res._midCellsPerLineRes = cplOK ? RFOk:RFNok;
	result &= cplOK;

	res._titer = estimateTiter(imginf._brightness, imginf._exposure, imginf._overexposure1, eptCF );

	return result;
}

bool PDImageProcessing::findHEp2Nucleus( QImage & img )
{
    QList<QRect>  nuclei;
    int level;
    return findHEp2Nucleus( img, level );
}


bool PDImageProcessing::findHEp2Nucleus1( QImage & img, int & nucleiIndex, int diamMin, int diamMax, QList<QRect> * nuclei )
{
try{	
	IplImage *orig = createGreyFromQImage(img);
	//cvShowImage("or", orig);
	IplImage* workimg = cvCreateImage( cvSize(orig->width/2,orig->height/2), orig->depth, orig->nChannels );
	cvPyrDown(orig,workimg);
	//cvShowImage("pd", workimg);
	cvNormalize(workimg,workimg,0,5,CV_MINMAX);
	cvThreshold( workimg, workimg, 1, 255,  CV_THRESH_BINARY );

	//BEGIN DEBUG ONLY	
	cvNormalize(workimg,workimg,0,255,CV_MINMAX);
	//cvShowImage("nrm1", workimg);
	//END 

	QList< QRect > cells;

	diamMin /= 2; //because of pyrDown, for faster processing. TODO consider scaling 4x
	diamMax /= 2;	
		
    CvMemStorage* storage = cvCreateMemStorage(0);
		

    
	//BEGIN DEBUG ONLY
	//cvShowImage("th", workimg);
	//END 
    CvSeq* contours = 0;
    

    cvFindContours( workimg, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	double cellsMean=0;
	double cellsAreaMean=0;
	double cellsAreaVar=0;
	QList <double> cellsAreas;

	CvScalar s = cvAvg( workimg );
	double imgAvg = s.val[0];

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
			    cells.append(QRect(contour->rect.x*2, contour->rect.y*2, contour->rect.width*2, contour->rect.height*2));
				cellsAreas.append( (contour->rect.height * contour->rect.width) );
		    }
	    }
    }

	int cellsNum = cells.size();

	do{

		nucleiIndex = 0;

		if ( cellsNum < 50 )
			break;

		cellsMean /= cellsNum; 
		if ( imgAvg > cellsMean) //Cells are darker then cytoplasma
			break;

		double meanValuePart = 33 - ( 0.33 * ( imgAvg / cellsMean ) *100 ); 
		nucleiIndex +=meanValuePart;

		double cellDensityIndex = cellsNum;
		cellDensityIndex /= 250; //about 250 cells are on entire area
		cellDensityIndex *= 100 * 0.33;
		nucleiIndex += cellDensityIndex;
		
		cellsAreaMean /= cellsNum;
		cellsAreaVar=0;
		foreach (double cav, cellsAreas)
		{
			cellsAreaVar += (cav-cellsAreaMean)*(cav-cellsAreaMean);
		}
		cellsAreaVar /= (cellsNum-1);
		cellsAreaVar = qSqrt(cellsAreaVar);	

		int withinSigma=0;

		double minArea = cellsAreaMean - cellsAreaVar/4;
		double maxArea = cellsAreaMean + cellsAreaVar/4;

		foreach (double cav, cellsAreas)
		{
			if ( cav > minArea &&  cav < maxArea )
				withinSigma++;
		}

		double similarNucleusPart = withinSigma;
		similarNucleusPart /= cellsAreas.size();
		similarNucleusPart *= 100 * 0.33;
		nucleiIndex +=similarNucleusPart;

		

	} while(0);

    cvReleaseMemStorage( &storage );    
	
	if(nuclei)
		*nuclei = cells;
	

	//todo: release seq!
	cvReleaseImage (&workimg );
	cvReleaseImage (&orig );	
}
catch (std::exception ex)
{
	return false;
}
	return nucleiIndex!=0;
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

int PDImageProcessing::findLocMaxima( QImage & img, int step, int minTh, QList<QRect> * positions )
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


int PDImageProcessing::estimateTiter( double brightness, double exposure, double overexposure, double correctionFctor )
{

	static const double TiterEstimationExposures[] = {-1,0,0.01,0.02,0.03,0.04,0.05,0.07,0.09,0.12,0.15,0.18,0.21,0.25,0.28,0.32,0.36,0.40};
	static const int TETabColumns = sizeof(TiterEstimationExposures)/sizeof(TiterEstimationExposures[0]);
	static const int TiterEstimationBrightnessValues[][TETabColumns] = {
		{260,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2},
		{240,-1,100,100,100,100,100,100,100,100,100,100,100,100,100,100,90,90},
		{230,-1,100,100,100,100,100,100,100,100,100,100,100,100,100,100,90,90},
		{220,-1,100,100,100,100,100,100,100,100,100,100,100,100,95,90,85,85},
		{210,-1,100,100,100,100,100,100,100,100,100,100,95,90,85,80,80,80},
		{200,-1,100,100,100,100,100,100,100,100,100,100,95,90,85,80,75,70},
		{190,-1,100,100,100,100,100,100,100,100,100,95,90,85,80,75,70,65},
		{180,-1,100,100,100,100,100,100,100,100,95,90,85,80,75,70,65,65},
		{170,-1,100,100,100,100,100,100,100,95,90,85,80,75,70,70,65,60},
		{160,-1,100,100,100,100,100,100,100,95,90,85,80,75,70,70,65,60},
		{150,-1,100,100,100,100,100,100,100,95,90,85,80,75,70,70,65,60},
		{140,-1,-1,100,100,100,100,100,95,90,85,80,75,70,70,65,60,55},
		{130,-1,-1,-1,100,100,100,100,95,90,85,80,75,70,70,65,50,55},
		{120,-1,-1,-1,-1,100,100,100,95,90,85,80,75,70,70,65,50,50},
		{110,-1,-1,-1,-1,-1,100,95,90,85,80,75,70,70,65,50,45,50},
		{100,-1,-1,-1,-1,-1,-1,95,90,85,80,75,70,70,65,50,45,45},
		{90,-1,-1,-1,-1,-1,-1,-1,90,85,80,75,70,70,65,45,40,45},
		{80,-1,-1,-1,-1,-1,-1,-1,-1,80,75,70,70,65,45,40,35,40},
		{70,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,30,35},
		{60,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,25,30},
		{50,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,25},
		{40,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,20},
		{30,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,5,10},
		{20,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0},
		{0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0},
		};

	int TETRows = (sizeof(TiterEstimationBrightnessValues) / sizeof(TiterEstimationBrightnessValues[0]));//sizeof(TiterEstimationBrightnessValues[0][0]);

	int ei=0;
	for (ei=TETabColumns-1; ei>=0; --ei)
	{
		if ( exposure >= TiterEstimationExposures[ei] )
			break;
	}

	int bi=0;
	for (bi=0; bi<=TETRows; ++bi) //todo use constant
	{
		int t = TiterEstimationBrightnessValues[bi][0];
		if ( brightness >= t )
		{
			int retValue = TiterEstimationBrightnessValues[bi][ei];
			retValue *= correctionFctor;
			if (retValue>100) 
				retValue=100;
			return retValue;
		}
	}

	return -1;
}

QPair<double, double> PDImageProcessing::getMeanAndDev ( const QList<double> list )
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

int PDImageProcessing::removeInhomogeniousAreas( QImage & img )
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

QMap<QByteArray,int> PDImageProcessing::getSequenceHistogram( QImage & img, int seqLen, int levels )
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

void PDImageProcessing::normalize( QImage & img )
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

int PDImageProcessing::getCytoplasmaIndex( QImage & img)
{
	IplImage *normImg = createGreyFromQImage(img);
	IplImage* out = cvCreateImage( cvSize(normImg->width/2,normImg->height/2), normImg->depth, normImg->nChannels );
	cvPyrDown(normImg,out);
	cvNormalize(out,out,0,5,CV_MINMAX);
	//cvShowImage("orig", out);
	QImage qimg = IplImage2QImage(out);
	
	double cytpix=0;
	double cytpix1=0;
	double cellpix=0;
	double cellpix1=0;
	double bgpix=0;
	for (int y = 0; y < qimg.height(); y++)
    {
	    for (int x = 0; x < qimg.width(); x++)
	    {
			unsigned char gv = (unsigned char) qGreen(qimg.pixel(x, y));
			if( gv == 0  )
			{
				bgpix +=1;
			}
			else if( gv == 1 )
			{
				cytpix +=1;
			}
			else if( gv == 2 )
			{
				cytpix1 +=1;
				cellpix1=0;
			}
			else if( gv > 2  )
			{
				cellpix +=1;
			}
			
		}
    }
	
	cytpix1 /=2;
	cytpix +=cytpix1;
	cellpix1 /=2;
	cellpix += cellpix1;

	double A=qimg.height() *qimg.width();

	double R_bg = bgpix / A;
	double R_cel = cellpix / A;
	double R_cyt = cytpix / A;

	double D_rc = (R_bg +R_cyt) / R_cel; //relative cell density

	double I_cyt1 = R_cyt / D_rc;


	cvNormalize(out,out,0,255,CV_MINMAX);
	//cvShowImage("orig1", out);
	cytpix /= qimg.height() *qimg.width();
	cytpix *=100;
	bgpix /= qimg.height() *qimg.width();
	bgpix *=100;
	return D_rc * 100;
}

bool PDImageProcessing::findHEp2Nucleus( QImage & imgIn, int & nucleiIndex, int diamMinIn, int diamMaxIn, QList<QRect> * nuclei )
{
	try{

	QMap<int, int> layerShapesNumber;
	QList< QList<QRect> > layerShapes;
	QList< QList<QRect> > layerShapesSmall;
	QImage img = ImageCache::instance().getImage(imgIn, QSize(640, 480) );
	int diamMin = diamMinIn / imgIn.width();
	diamMin /= 640;
	int diamMax = diamMaxIn / imgIn.width();
	diamMax /= 640;
	int width = img.width();
	int height = img.height();
	CvSize Size;
	Size.height = height;
	Size.width = width;
	IplImage *g_gray = cvCreateImage(Size, IPL_DEPTH_8U, 1);
	
	PDImageInfo imgInfo;
	getImageInfo( img, imgInfo );
	double meanPixValueOrig = imgInfo._meanPixValue;
	double meanPixFgValueOrig = imgInfo._meanFgPixValue;

	removeBackground( img, imgInfo._bgLevel1 );
	QImage noNucleiImage = img;

	IplImage *normImg = createGreyFromQImage(img);
	cvEqualizeHist (normImg , normImg );
	//cvErode (normImg , normImg );
	// get the image data

	/*inversion of image
	  height    = normImg->height;
	    width     = normImg->width;
	int     step      = normImg->widthStep;
	int    channels  = normImg->nChannels;
	uchar *    data      = (uchar *)normImg->imageData;
	 	 
	  // invert the image
	  for(int i=0;i<height;i++)
		 for(int j=0;j<width;j++)
			for(int k=0;k<channels;k++)  //loop to read for each channel
			   data[i*step+j*channels+k]=255-data[i*step+j*channels+k];    //inverting the image
    */
	

	img = IplImage2QImage(normImg);

	//getImageInfo( img, imgInfo );
		
    CvMemStorage* storage = cvCreateMemStorage(0);
	int bestLayer=0;
	int layer=0;
	int bestLayerValue=-1;

	//int layerThkn=10;	
	int step=5;	
	//for (int th=255; th > 60; th-=layerThkn )
	for (int th=200; th > 60; th -= 5 )
    {
		layerShapes.append( QList<QRect>() );
		layerShapesSmall.append( QList<QRect>() );
        unsigned char *charTemp = (unsigned char *) g_gray->imageData;
	    for (int y = 0; y < height; y++)
	    {
		    for (int x = 0; x < width; x++)
		    {
			    int index = y * width + x;
				unsigned char gv = (unsigned char) qGreen(img.pixel(x, y));
				//if ( gv > th && gv < (th+layerThkn) )
				if ( gv > th )
					charTemp[index] = 255;
				else
					charTemp[index] = 0;
			}
	    }

        //cvThreshold( g_gray, g_gray, th, 255,  CV_THRESH_BINARY );
	    CvSeq* contours = 0;
	    

	    cvFindContours( g_gray, storage, &contours, sizeof(CvContour),
                        CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
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
				    layerShapes[layerShapes.size()-1].append(QRect(contour->rect.x, contour->rect.y, contour->rect.width, contour->rect.height));
			    }
				//TODO activate using param
				/*else if ( //smaller contours in and around cell //TODO: use variables                    
                     ( contour->rect.height >= 3 && contour->rect.height <= 10 )
				      &&( contour->rect.width >= 3 && contour->rect.width <= 10 ) 
                   )
			    {
				    layerShapesSmall[layerShapesSmall.size()-1].append(QRect(contour->rect.x, contour->rect.y, contour->rect.width, contour->rect.height));
			    }*/
		    }
			if ( layerShapes[layerShapes.size()-1].size() > bestLayerValue )
			{
				bestLayerValue = layerShapes[layerShapes.size()-1].size();
				bestLayer = layer;
			}
  	    }
		++layer;
    }	

	

    cvReleaseMemStorage( &storage );
	
    QPainter p1(&imgIn);
    QPen pen(Qt::SolidLine);
    pen.setWidth(1);
    pen.setColor(QColor("blue"));
	p1.setPen( pen );
	
	QPainter p(&noNucleiImage);
	p.setBrush( Qt::black );
	p.setPen( Qt::black );

 
	if (bestLayer>=0)
	foreach (QRect r, layerShapes[ bestLayer ])
	{	
	    p.drawEllipse(r);

		QRect oririnalRect(r.x()*4, r.y()*4, r.width()*4, r.height()*4  );
		if( nuclei )
			nuclei->append(oririnalRect);


		p1.drawEllipse(oririnalRect );
	}


	getImageInfo( noNucleiImage, imgInfo );
	double meanPixValueErased = imgInfo._meanPixValue;
	double meanFgPixValueErased = imgInfo._meanFgPixValue;

	if (meanPixValueOrig <= 1)
		nucleiIndex = 100;
	else
		nucleiIndex= ( meanPixValueErased / meanPixValueOrig ) * 100;

	cvReleaseImage (&g_gray );
	cvReleaseImage (&normImg );
}
catch (std::exception ex)
{
	return false;
}
	//imgIn = img.scaled(imgIn.size());
	return nucleiIndex!=0;
}

QList<QRect> PDImageProcessing::findAncaes( QImage & img )
{
	QList<QRect> ancaes;
	try{
		int width = img.width();
		int height = img.height();
		CvSize Size;
		Size.height = height;
		Size.width = width;
		IplImage *g_gray = cvCreateImage(Size, IPL_DEPTH_8U, 1);
		
		PDImageInfo imgInfo;
		getImageInfo( img, imgInfo );

		char *charTemp = (char *) g_gray->imageData;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int index = y * width + x;
				charTemp[index] = (char) qGreen(img.pixel(x, y));
			}
		}		
		
		cvThreshold( g_gray, g_gray, imgInfo._bgLevel, 255, CV_THRESH_BINARY );
		cvAdaptiveThreshold(g_gray, g_gray, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 43, 0);
		CvSeq* contours = 0;
		CvMemStorage* storage = cvCreateMemStorage(0);

		cvFindContours( g_gray, storage, &contours, sizeof(CvContour),
						CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
		if ( contours )
		{
			for (CvSeq*cont = contours; cont != 0; cont = cont->h_next)
			{
				CvContour  *contour = (CvContour *)cont;
				if (  ( contour->rect.height >= 60 && contour->rect.height <= 120 )
					&&( contour->rect.width >= 60 && contour->rect.width <= 120 ) )
				{
					QPainter p(&img);
					QPen pen(Qt::SolidLine);
					pen.setWidth(5);
					pen.setColor(QColor("blue"));
					p.setPen( pen );
					p.drawEllipse(QRect(contour->rect.x, contour->rect.y, contour->rect.width, contour->rect.height ));
					ancaes.append(QRect(contour->rect.x, contour->rect.y, contour->rect.width, contour->rect.height));
				}
			}
		}

		cvReleaseImage (&g_gray );
	}
	catch (std::exception ex)
	{
		
	}
	return ancaes;
}

bool initCV()
{
	cvNamedWindow( "test", CV_WINDOW_AUTOSIZE );
	return 0;
}

//TODO: Experimental code for pattern matching. Move in separate function
QList<QRect> PDImageProcessing::findItems( QImage & imgIn, QImage tmplIn, int meth )
{
	IplImage * timg = createGreyFromQImage(imgIn);	
	IplImage * ttmpl = createGreyFromQImage(tmplIn);

	cv::Mat img = cv::cvarrToMat( timg, true );
	cv::Mat tmpl = cv::cvarrToMat( ttmpl, true );
	cvReleaseImage (&timg );
	cvReleaseImage (&ttmpl );
	return findItems( img, tmpl, meth );

}
QList<QRect> PDImageProcessing::findItems( cv::Mat &img, cv::Mat &tmpl, int meth )
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

QList<QPolygon> PDImageProcessing::getHuMoments( QImage & imgIn, int minsize, int maxsize, double* hu )
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

Mat normalizeCel( Mat cell, RotatedRect rr )
{
	//create bigger scene for working with
	Mat working_scene =  Mat::zeros( cell.size()*2, CV_8UC1 );
	
	//copy cell in scene using RIO
	cv::Rect cellAreaRect = cv::Rect( working_scene.size().width/4, working_scene.size().height/4,working_scene.size().width/2, working_scene.size().height/2);
	cv::Mat cellArea = working_scene(cellAreaRect);
	cell.copyTo(cellArea);

	Point center = Point( working_scene.size().width/2, working_scene.size().height/2 );
    double angle = rr.angle;
    double scale = 1;

    /// Get the rotation matrix with the specifications above
    Mat rot_mat = getRotationMatrix2D( center, angle, scale );

   /// Rotate the warped image
   warpAffine( working_scene, working_scene, rot_mat, working_scene.size() );

   vector<vector<Point> > contours;

   Mat canny = Mat::zeros( working_scene.rows, working_scene.cols, CV_8UC1 );
   Canny(working_scene, canny,10, 255);

   findContours(canny, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);   
   Mat mask = Mat::zeros(working_scene.rows, working_scene.cols, CV_8UC1);

   Rect br;
   for( int i = 0; i < contours.size(); i++ )
	{ 
		if( contours[i].size() < 5  )
			continue;
		br = minAreaRect( Mat(contours[i]) ).boundingRect();
		if (( br.size().width >= cell.size().width/2 )
			&&
			( br.size().height >= cell.size().height/2 ) )
		{
			drawContours(mask, contours, i, Scalar(255), CV_FILLED);
			break;
		}
	}

  int px = 2;   
  Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                      cv::Size(2 * px + 1, 2 * px + 1), 
                      cv::Point(px, px) );
  Mat ret;
  erode(mask,mask,kernel);
  dilate(mask,mask,kernel);
  dilate(mask,mask,kernel); 
  if( mask.size().width==0 || mask.size().height ==0 )
	return ret;

  working_scene.copyTo(working_scene,mask);

  
  if (!working_scene.data)
	  return ret;

  ret = working_scene( br );

  Point2f srcTri[3];
  Point2f dstTri[3];
  srcTri[0] = Point2f( 0,0 );
   srcTri[1] = Point2f( ret.cols - 1, 0 );
   srcTri[2] = Point2f( 0, ret.rows - 1 );

   double wf = 60.0 / br.size().width;
   double hf = 60.0 / br.size().height;


   //dstTri[0] = Point2f( src.cols*0.0, src.rows*0.33 );
   //dstTri[1] = Point2f( src.cols*0.85, src.rows*0.25 );
   //dstTri[2] = Point2f( src.cols*0.15, src.rows*0.7 );

   dstTri[0] = Point2f( 0,0);
   dstTri[1] = Point2f( ret.cols*wf , 0 );
   dstTri[2] = Point2f( 0,ret.rows *hf );

		   

   Mat warp_mat( 2, 3, CV_32FC1 );
   /// Get the Affine Transform
   warp_mat = getAffineTransform( srcTri, dstTri );

   /// Apply the Affine Transform just found to the src image
   warpAffine( ret, ret, warp_mat, Size(60,60) );
	
  return ret;
}


QList<CellInfo> PDImageProcessing::getCells( QImage & imgIn, int minsize, int maxsize )
{
	QList<CellInfo> cells;
	QImage  img = imgIn;//.scaled( imgIn.width(), imgIn.height() );
	//diffXY( img, 5 ); // 1st derivation for better extraction of contours	
	IplImage * g_gray = createGreyFromQImage(img);

	Mat  src_gray= cv::cvarrToMat( g_gray, true );
	cvReleaseImage (&g_gray );

	Mat img1 = Mat::zeros( src_gray.rows, src_gray.cols, CV_8UC1 );

    // apply your filter
    Canny(src_gray, img1,10, 255);


	QList<QPolygon> ellipses;



    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
	Mat drawing = Mat::zeros( src_gray.rows, src_gray.cols, CV_8UC1 );

  /// Detect edges using Threshold
  //threshold( src_gray, threshold_output, 1, 255, THRESH_BINARY );
  /// Find contours
  //findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
   findContours(img1, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);   
   Mat mask = Mat::zeros(src_gray.rows, src_gray.cols, CV_8UC1);
   //drawContours(mask, contours, -1, Scalar(255), CV_FILLED);  
  for( int i = 0; i < contours.size(); i++ )
	{ 
		if( contours[i].size() < 5  )
			continue;
		RotatedRect rr = minAreaRect( Mat(contours[i]) );
		if (( rr.size.width >= minsize && rr.size.width <= maxsize  )
			&&
			( rr.size.height >= minsize && rr.size.height <= maxsize  ) )
		{
			drawContours(mask, contours, i, Scalar(255), CV_FILLED);
		}
	}

  int px = 2;   
  Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                      cv::Size(2 * px + 1, 2 * px + 1), 
                      cv::Point(px, px) );
  erode(mask,mask,kernel);
  dilate(mask,mask,kernel);
  dilate(mask,mask,kernel);  


///
   Point2f srcTri[3];
   Point2f dstTri[3];

   Mat rot_mat( 2, 3, CV_32FC1 );
   Mat warp_mat( 2, 3, CV_32FC1 );
   Mat warp_dst, rotate_dst;

 
  


  findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
  mask = Mat::zeros(src_gray.rows, src_gray.cols, CV_8UC1);
  for( int i = 0; i < contours.size(); i++ )
	{ 
		if( contours[i].size() < 5  )
			continue;

		Mat mat = Mat(contours[i]);
		
		
		RotatedRect ce = fitEllipse( mat );
		Point2f cepts[4];
		ce.points( cepts );
		RotatedRect rr = ce;//minAreaRect( mat );
		cv::Rect br = boundingRect(mat);
		//cv::Rect br = ce.boundingRect();
		if (( rr.size.width >= minsize && rr.size.width <= maxsize  )
			&&
			( rr.size.height >= minsize && rr.size.height <= maxsize  )
			&&
			(br.x>0 && br.y > 0)
			&&
			(br.x+br.width<src_gray.rows && br.y+br.height < src_gray.cols)
			)
		{
			
			
			
			
			ellipses.append(QRect(br.x,br.y,br.width,br.height));
			Mat cell = src_gray(br);
			if ( cell.size().width <=0 || cell.size().height <= 0 )
				continue;
			Mat normcell = normalizeCel(cell,ce);
			if (!normcell.data)
			{
				continue;
			}

			cv::Mat cellArea;
			cellArea= drawing(br);
			normcell.copyTo(cellArea);

			//Moments m =  moments(src);
			//Point2f mc = Point2f( m.m10/m.m00 , m.m01/m.m00 );
			//ellipses.append(QRect(br.x +mc.x-2,br.y +mc.y-2,4,4));
		    //Scalar color = Scalar( 155,155,155 );
		    //drawContours( mask, contours, i, color, 2, 8, hierarchy, 0, Point() );
			drawContours(mask, contours, i, Scalar(255), CV_FILLED);			  		

			//imshow( "Cell", normcell );

			Mat features(32,32,CV_8UC1);
			resize(normcell, features, features.size());
			//imshow( "CellF", features );

			CellInfo ci;
			//ci._normalized = QImage( normcell.data, normcell.size().width(), normcell.size().height(),normcell.size().width(),QImage::Format_RGB888 ;
			QString svminfo;

			uchar* data = features.data;

			for(int i = 0; i < 32*32; i++)
			{
				svminfo +=QString( "%1:%2 " ).arg(i+1).arg((int)*data);
			    data++;
			}

			Rect cer = ce.boundingRect();
			ci._rrect = QRect(cer.x, cer.y, cer.width, cer.height);
			ci._svmentry = svminfo;

			cells.append(ci);

			//imshow( "Cell_w", warp_dst );
			//imshow( "Cell_wt", rotate_dst );

		}
	}

 
  //imshow( "Cells", drawing );
  //imshow( "Mask", mask );

  return cells;

}

QList<QPolygon> PDImageProcessing::findEllipses( QImage & imgIn, int minsize, int maxsize )
{
	return findEllipses1( imgIn, minsize, maxsize );
}

QList<QPolygon> PDImageProcessing::findEllipses1( QImage & imgIn, int minsize, int maxsize )
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

double PDImageProcessing::findPeaks( QImage & img, int diamMin, int diamMax, int levelOf5, QList<QRect> * peaks )
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


bool PDImageProcessing::classifyImage( QImage &img, int layer )
{	
	PDImageInfo imginf;
	getImageInfo( img, imginf );
	QRect tile;
	if (tile.isEmpty())
	{
		tile = QRect(QPoint(img.width(),img.height()), QSize(img.width(), img.height() ) );
	}

	int focusHeight = tile.height();
	int focusWidth  = tile.width();
	int focusPosX = 0;
	int focusPosY = 0;	    

	//int q=focusWidth/4;
	int focusPosXTo = focusPosX + focusWidth;
	int focusPosYTo = focusPosY + focusHeight;

	QImage store = img.copy(focusPosX, focusPosY, focusPosXTo-focusPosX, focusPosYTo-focusPosY );


		//int max=0;
		int lr=(focusPosXTo-focusPosX)/1;
		QVector<int> locVal( lr );	
		//int histwidth = (imginf._histogramMax - imginf._histogramMin)/2;
		for (int y =focusPosY; y < focusPosYTo; ++y)	
		{		
			for (int x =focusPosX; x < focusPosXTo; ++x)
			{			
				unsigned char c = img.pixel(x,y) >> 8;

				if ( c > layer )
				{				
					//img.setPixel( x, y, 0xFFFFFFFF  );
                    //store.setPixel( x, y, 0xFFFFFFFF  );
				}
                else{
					//img.setPixel( x, y, 0x0 );
                    store.setPixel( x, y, 0x0 );
                }
			}
		}	
		
	store.save("img.png");
	IplImage* cimg = cvLoadImage("img.png");
	IplImage*g_gray = cvCreateImage( cvGetSize( cimg ), 8, 1 );
	cvCvtColor( cimg, g_gray, CV_BGR2GRAY );
//	cvSmooth(g_gray, g_gray, CV_GAUSSIAN, 7, 7); 



	CvSeq* contours = 0;
	CvMemStorage* storage = cvCreateMemStorage(0);

	cvFindContours( g_gray, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	if ( contours )
	{
		const char* attrs[] = {"recursive", "1", 0};
		cvSave("contours.xml", contours, 0, 0, cvAttrList(attrs, 0));

		for (CvSeq*cont = contours; cont != 0; cont = cont->h_next)
		{
			CvContour  *contour = (CvContour *)cont;
			if (  ( contour->rect.height >= 40 && contour->rect.height <= 150 )
				&&( contour->rect.width >= 40 && contour->rect.width <= 150 ) )
			{
				QPainter p(&img);
				p.setPen( QColor("blue"));
				p.drawEllipse(QRect(contour->rect.x, contour->rect.y, contour->rect.width, contour->rect.height ));
			}
		}
	}

	cvZero( g_gray );
	if( contours ){
		cvDrawContours(
			g_gray,
			contours,
			cvScalarAll(255),
			cvScalarAll(255),
			1,1 );
	}
	//cvShowImage( "Contours", g_gray );

	cvReleaseMemStorage( &storage );
    cvReleaseImage( &cimg );
	cvReleaseImage (&g_gray );

	return true;
}

bool PDImageProcessing::merge( QImage &imgout, const QImage &imgAdd )
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
qint32 PDImageProcessing::calculateFocusValue( QImage & img, int alg, QRect tile, bool debugMode )
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

void PDImageProcessing::diffX( QImage & img, int th )
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

void PDImageProcessing::diffY( QImage & img, int th )
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

void PDImageProcessing::diffXY( QImage & img, int th )
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


void PDImageProcessing::overlayImageInfo( QImage &cimg, const QPoint & pos  )
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

void PDImageProcessing::reduceBackgroundNoise( QImage &img, int bgvalue )
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

void PDImageProcessing::removeBackground( QImage &img, int bgvalue )
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



void PDImageProcessing::getImageInfo( QImage &img, PDImageInfo & imgInfo )
{
try{
	if ( PDImageInfo::ExpectedImageType == EICT_ANCAE || PDImageInfo::ExpectedImageType == EICT_ANCAF )
	{
		getImageInfoANCA( img, imgInfo ); //TODO: check using better new function for all types
		return;
	}

	


	QRect roi(QPoint(0,0),img.size());
	//if( img.width() > 1024 && img.height() > 768 )
	//	roi = QRect( QPoint(img.width() /2 - 1024/2, img.height()/2 - 768/2), QSize(1024, 768) );

    QImage cimg = img.copy(roi);
    //TODO logging not thread-safe PD_TRACE("getImageInfo");
    imgInfo.reset();

	if ( cimg.isNull() || cimg.height()<=0 || cimg.width()<=0 )
	{
		//TODO logging not thread-safe PD_WARNING( "Requesting image information on null image" );
		return;
	}

	imgInfo._exposure = img.text("exposure").toDouble();
	//calculating histogram and average pixel value
	const int lines = cimg.height();
	const int pixPerLine = cimg.width();
	const int bw =  lines/20;
	const int bh =  pixPerLine/20;
	if ( bw<1 || bh<1 )
	{
		return;//false
	}
	int overexposedPixels[21][21]={0};
	int l=0, p=0;
	QSize is = cimg.size();
    //TODO logging not thread-safe PD_TRACE("bw %d,; bh%d; lines %d; pixPerLine %d",bw,bh, lines, pixPerLine);
    int hmax=255, hmin=0;
	

	int oeth=10;//over exposed size threshold

    for (l=0; l< lines; ++l)
	{
		QRgb* pix = (QRgb*)cimg.scanLine(l);
		unsigned char gp=0;
		int il = l/bw;
		bool skipOePix=false;
		for (p=0; p<pixPerLine; p++)
		{
            Q_ASSERT(bh!=0);
			int ic = p/bh;
			gp = qGreen(*pix);
			++imgInfo._histogram[ gp ];
			imgInfo._meanPixValue += gp;
			pix++;
			if ( gp == 255 )
			{
				for (int i=0; i<oeth && p<pixPerLine; ++i, ++p)
		{
			gp = qGreen(*pix);
			++imgInfo._histogram[ gp ];
			imgInfo._meanPixValue += gp;
			++pix;
					if(gp < 255)
					{
						skipOePix=true;
						break;
					}
				}
				if (!skipOePix)
			{
                if ( il >20 || ic >20 )
                {
                    //PD_ERROR( "Array boundary violation [%d][%d]", il, ic )
                    continue;
                }
                else
                {
						overexposedPixels[il][ic]+=oeth;
                }
			}
		}
		}
	}
    //TODO logging not thread-safe PD_TRACE("imgInfo: %d,%d,%d,%d",imgInfo._bgLevel, imgInfo._histogramMax, imgInfo._histogramMin, imgInfo._meanPixValue);

    int bg= 0;
	for (bg = 3; bg<255; ++bg )
	{
		if ( (imgInfo._histogram[bg-1] + imgInfo._histogram[bg]) 
                > 
             (imgInfo._histogram[bg-2] +  imgInfo._histogram[bg-3])
        )
		break;
	}
    imgInfo._bgLevel = (bg > 127 )? 40:( bg+10); //prevent error-determination. reset to 40 (reasonable value)

	int dState=0;
    int bgcut1=0;
    for (bg = 3; bg<255; ++bg ) // iterate over histogram
	{
		if ( dState==0 )
		{
			if ( (imgInfo._histogram[bg-1] + imgInfo._histogram[bg]) 
					> 
				 (imgInfo._histogram[bg-2] +  imgInfo._histogram[bg-3])
			)
			{
			dState=1;
			}
			bgcut1=bg;
		}
		else if( dState==1 )
		{
			if ( (imgInfo._histogram[bg-1] + imgInfo._histogram[bg]) 
					< 
				 (imgInfo._histogram[bg-2] +  imgInfo._histogram[bg-3])
			)
			break;
		}
	}
    imgInfo._bgLevel1= bg;
    if ( imgInfo._bgLevel1 == 0  )
	{
		imgInfo._bgLevel1 = 20;
	}
	else if ( (bg - bgcut1) > 10  )
	{
		imgInfo._bgLevel1 = bgcut1 + 5;
	}
    else if ( bg > 127  )
	{
		imgInfo._bgLevel1 = 30;
	}
	
    int bgpixels=0;
    for (int i = 0; i<=imgInfo._bgLevel1; ++i )
	{
		bgpixels += imgInfo._histogram[i];
	}

    imgInfo._bgRatio = (((double)bgpixels)/(cimg.height() * cimg.width()))*100;
    if (imgInfo._bgRatio >= 99)
    {
        imgInfo._histogramWidth=0;
        imgInfo._histogramMax = imgInfo._bgLevel;
        imgInfo._histogramMin = imgInfo._bgLevel;
        imgInfo._meanFgPixValue = 0;
        imgInfo._meanPixValue /= cimg.height() * cimg.width();
//        imgInfo._overexposure=0;
        imgInfo._overexposure1 = 0;
    }
    else
    {
        for (int i = imgInfo._bgLevel1; i<=255; ++i )
	    {
		    imgInfo._meanFgPixValue += i * imgInfo._histogram[i];
	    }

        int fgpixels = (cimg.height() * cimg.width()) - bgpixels;
        if ( fgpixels > 0 )
        {
            imgInfo._meanFgPixValue /= fgpixels;
        }
        else if( fgpixels < 0 )
        {
            //TODO logging not thread-safe PD_ERROR ("Internal error. Invalid FG value")
        }
    //     
        unsigned long threshold = ((cimg.height() * cimg.width()) - bgpixels)/1000; 
        
        threshold = 1+ qSqrt( threshold );
        


        //TODO logging not thread-safe PD_TRACE("threshold %d",threshold);

        for (hmax=255; hmax>0; --hmax )
	    {
		    if ( imgInfo._histogram[hmax] > threshold )
            {
                if ( hmax > 1 && imgInfo._histogram[hmax-1] > threshold )
			        break;
            }
	    }
        for (hmin = 0; hmin<255; ++hmin )
	    {
		    if ( imgInfo._histogram[hmin] > threshold )
			    break;
	    }
    	

	    imgInfo._histogramMin = hmin;
	    imgInfo._histogramMax = hmax;

        imgInfo._histogramWidth = imgInfo._histogramMax - imgInfo._bgLevel;

        

//	    imgInfo._overexposure=0;
        int overexposed=0;
		int overexposedBlocks=0;
	    for (int l=0; l<20; ++l)
	    {
		    for (int c=0; c<20; ++c)
		    {
                overexposed += overexposedPixels[l][c];
			    if ( overexposedPixels[l][c] > 10 )
			    {
//				    imgInfo._overexposure+=10; //todo: +10 for fitting the value into previous order.
					overexposedBlocks++;
			    }
		    }
	    }
        int nonbgpixs = (cimg.height() * cimg.width()) - bgpixels;
        imgInfo._overexposure1 = (((double)overexposed) / nonbgpixs ) * 10000; // 10th of per mil
    
		//if ( overexposedBlocks < 40 ) //Less then 10% overexposed
		//	imgInfo._overexposure1=0;
    
        //TODO logging not thread-safe PD_TRACE("imgInfo.oexp: %d",imgInfo._overexposure);



	    imgInfo._meanPixValue /= cimg.height() * cimg.width();

	    //calculating brightness using only values above histogram middle value - values higher than 2/3
	    int brightPixels=0;
	    int brightPixelsMeanVal=1;
	    int histAbsMidValue = imgInfo._histogramMax;
	    histAbsMidValue *=2;
	    histAbsMidValue /=3; //using upper 2/3 //Test 1/2
		//TEST: 3/4 was worse!!! consider only brightest


		double pixels = cimg.height() * cimg.width();
		double minBrightArea = pixels / 1500; // 0,075%

		for ( int i=imgInfo._histogramMax; i > 0; --i )
	    {
		    brightPixels+=imgInfo._histogram[i];
		    brightPixelsMeanVal+=imgInfo._histogram[i] * i;
			if ( brightPixels >= minBrightArea )
				break;
	    }

	    /*for ( int i=histAbsMidValue; i<=imgInfo._histogramMax; ++i )
	    {
		    brightPixels+=imgInfo._histogram[i];
		    brightPixelsMeanVal+=imgInfo._histogram[i] * i;
	    }
		*/

		//double pixels = cimg.height() * cimg.width();

		//double brightPixelsPct = (((double)brightPixels) / pixels) * 100;

	    //if ( brightPixelsPct > 2  ) // if area of bright pixels exceeds 2%
	    {
			if ( brightPixels <= 0  )
				brightPixels = 1;
		    imgInfo._brightness = brightPixelsMeanVal / brightPixels; 
	    }
		//else
		{
		//	imgInfo._brightness = imgInfo._meanPixValue;
		}
    }
	}
	catch(std::exception ex)
	{
		std::string what = ex.what();
	}
}

void PDImageProcessing::getImageInfoANCA( QImage &img, PDImageInfo & imgInfo )
{
	QRect roi(QPoint(0,0),img.size());
	//if( img.width() > 1024 && img.height() > 768 )
	//	roi = QRect( QPoint(img.width() /2 - 1024/2, img.height()/2 - 768/2), QSize(1024, 768) );

    QImage cimg = img.copy(roi);
    //TODO logging not thread-safe PD_TRACE("getImageInfo");
    imgInfo.reset();

	if ( cimg.isNull() || cimg.height()<=0 || cimg.width()<=0 )
	{
		//TODO logging not thread-safe PD_WARNING( "Requesting image information on null image" );
		return;
	}
    
	//calculating histogram and average pixel value
	const int lines = cimg.height();
	const int pixPerLine = cimg.width();
	const int bw =  lines/20;
	const int bh =  pixPerLine/20;
	if ( bw<1 || bh<1 )
	{
		return;//false
	}
	int overexposedPixels[21][21]={0};
	int l=0, p=0;
	QSize is = cimg.size();
    //TODO logging not thread-safe PD_TRACE("bw %d,; bh%d; lines %d; pixPerLine %d",bw,bh, lines, pixPerLine);
    int hmax=255, hmin=0;
	

	int oeth=5;//over exposed size threshold
	int oeCnt=0;
	double overexposed=0;

    for (l=0; l< lines-1; ++l) //VH lines-1 because last line seems to often have garbage pixels
	{
		QRgb* pix = (QRgb*)cimg.scanLine(l);
		unsigned char gp=0;
		int il = l/bw;

		for (p=0; p<pixPerLine-1; p++)
		{
            Q_ASSERT(bh!=0);
			int ic = p/bh;
			gp = qGreen(*pix);
			++imgInfo._histogram[ gp ];
			imgInfo._meanPixValue += gp;
			pix++;
			if ( gp == 255 )
			{
				overexposed+=1;
				oeCnt++;
				if(oeCnt>oeth)
				{
					overexposedPixels[il][ic]+=1;
					oeCnt=0;
				}
			}
			else
			{
				oeCnt=0;
			}	
		}
	}
    //TODO logging not thread-safe PD_TRACE("imgInfo: %d,%d,%d,%d",imgInfo._bgLevel, imgInfo._histogramMax, imgInfo._histogramMin, imgInfo._meanPixValue);

    int bg= 0;
	for (bg = 3; bg<255; ++bg )
	{
		if ( (imgInfo._histogram[bg-1] + imgInfo._histogram[bg]) 
                > 
             (imgInfo._histogram[bg-2] +  imgInfo._histogram[bg-3])
        )
		break;
	}
    imgInfo._bgLevel = (bg > 127 )? 40:( bg+10); //prevent error-determination. reset to 40 (reasonable value)

	int dState=0;
    int bgcut1=0;
    for (bg = 3; bg<255; ++bg ) // iterate over histogram
	{
		if ( dState==0 )
		{
			if ( (imgInfo._histogram[bg-1] + imgInfo._histogram[bg]) 
					> 
				 (imgInfo._histogram[bg-2] +  imgInfo._histogram[bg-3])
			)
			{
				dState=1;
			}
			bgcut1=bg;
		}
		else if( dState==1 )
		{
			if ( (imgInfo._histogram[bg-1] + imgInfo._histogram[bg]) 
					< 
				 (imgInfo._histogram[bg-2] +  imgInfo._histogram[bg-3])
			)
			break;
		}
	}
    imgInfo._bgLevel1= bg;
    if ( imgInfo._bgLevel1 == 0  )
	{
		imgInfo._bgLevel1 = 20;
	}
	else if ( (bg - bgcut1) > 10  )
	{
		imgInfo._bgLevel1 = bgcut1 + 5;
	}
    else if ( bg > 127  )
	{
		imgInfo._bgLevel1 = 30;
	}

    int bgpixels=0;
    for (int i = 0; i<=imgInfo._bgLevel1; ++i )
	{
		bgpixels += imgInfo._histogram[i];
	}

    imgInfo._bgRatio = (((double)bgpixels)/(cimg.height() * cimg.width()))*100;
    if (imgInfo._bgRatio >= 99)
    {
        imgInfo._histogramWidth=0;
        imgInfo._histogramMax = imgInfo._bgLevel;
        imgInfo._histogramMin = imgInfo._bgLevel;
        imgInfo._meanFgPixValue = 0;
        imgInfo._meanPixValue /= cimg.height() * cimg.width();
        imgInfo._overexposure1 = 0;
    }
    else
    {
        for (int i = imgInfo._bgLevel1; i<=255; ++i )
	    {
		    imgInfo._meanFgPixValue += i * imgInfo._histogram[i];
	    }

        int fgpixels = (cimg.height() * cimg.width()) - bgpixels;
        if ( fgpixels > 0 )
        {
            imgInfo._meanFgPixValue /= fgpixels;
        }
        else if( fgpixels < 0 )
        {
            //TODO logging not thread-safe PD_ERROR ("Internal error. Invalid FG value")
        }
    //     
        unsigned long threshold = ((cimg.height() * cimg.width()) - bgpixels)/1000; 
        
        threshold = 1+ qSqrt( threshold );
        


        //TODO logging not thread-safe PD_TRACE("threshold %d",threshold);

        for (hmax=255; hmax>0; --hmax )
	    {
		    if ( imgInfo._histogram[hmax] > threshold )
            {
                if ( hmax > 1 && imgInfo._histogram[hmax-1] > threshold )
			        break;
            }
	    }
        for (hmin = 0; hmin<255; ++hmin )
	    {
		    if ( imgInfo._histogram[hmin] > threshold )
			    break;
	    }
    	

	    imgInfo._histogramMin = hmin;
	    imgInfo._histogramMax = hmax;

        imgInfo._histogramWidth = imgInfo._histogramMax - imgInfo._bgLevel;

        

		int overexposedBlocks=0;
	    for (int l=0; l<20; ++l)
	    {
		    for (int c=0; c<20; ++c)
		    {
			    if ( overexposedPixels[l][c] )
			    {
					overexposedBlocks++;
			    }
		    }
	    }

		if ( imgInfo._overexposure1>1 && overexposedBlocks < 40 ) //Less then 5% overexposed
		{
			imgInfo._overexposure1=0;
		}
		else
		{
			double oeRatio = ( overexposed / ( cimg.height() * cimg.width() ))*100;
			if( oeRatio >=2 )
				imgInfo._overexposure1 = 1000;
			else if( oeRatio >=1 )
				imgInfo._overexposure1 = 500;
			else if( oeRatio >=.5 )
				imgInfo._overexposure1 = 100;
			else if( oeRatio >=.2 )
				imgInfo._overexposure1 = 20;
			else if( oeRatio >=.12 )
				imgInfo._overexposure1 = 5;
			else if( oeRatio >=.09 )
				imgInfo._overexposure1 = 2;
			else if( oeRatio >=.06 )
				imgInfo._overexposure1 = 1;
			else 
				imgInfo._overexposure1 = 0;
		}			
    
        //TODO logging not thread-safe PD_TRACE("imgInfo.oexp: %d",imgInfo._overexposure);



	    imgInfo._meanPixValue /= cimg.height() * cimg.width();

	    //calculating brightness using only values above histogram middle value - values higher than 2/3
	    int brightPixels=0;
	    int brightPixelsMeanVal=1;
	    int histAbsMidValue = imgInfo._histogramMax;
	    histAbsMidValue *=2;
	    histAbsMidValue /=3; //using upper 2/3

	    for ( int i=histAbsMidValue; i<=imgInfo._histogramMax; ++i )
	    {
		    brightPixels+=imgInfo._histogram[i];
		    brightPixelsMeanVal+=imgInfo._histogram[i] * i;
	    }
	    if ( brightPixels > 0  )
	    {
		    imgInfo._brightness = brightPixelsMeanVal / brightPixels; 
	    }
    }
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
