/* ===========================================================================
 * Copyright 2015: Valentin Heinitz, www.heinitz-it.de
 * Tools for image processing
 * Author: Valentin Heinitz, 2015-12-12
 * License: MIT
 *
 ========================================================================== */
#ifndef IMAGE_PROCESSING_HG_
#define IMAGE_PROCESSING_HG_
#include <QImage>
#include <QString>
#include <QStringList>
#include <QMutex>
#include <QRunnable>

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

namespace cv{
		class Mat;
	};

#if 0



class CLIB_EXPORT ImgProc
{
public:
	enum ChannelMode { Red, Green, Blue, RGB, Grey1Ch, Grey3Ch };
	static cv::Mat toMat( QImage img, ChannelMode in=Green, ChannelMode out=Grey1Ch );
	static QImage toQImage( const cv::Mat &src );

	static QImage analyseStripe(QImage qi);

	static void rectify(QImage &img, QList<QPoint>, QRectF *br=0);
	static void storeImageTask(QString path, QImage img, int beautify, TImageMap, QObject *callback=0);
	static bool storeImageTaskBusy(int &readypct); ///<Returns true, if any store image thread is still running in backbround
   
	static void beautifyImage(QString img);
	static void reduceBgNoise(QImage &img);
	static void sharpen(QImage &img);
	//static void inpaint( QImage &img );
	static double compareHistogramm(QImage &img1, QImage &img2, int, QRect r = QRect());	
	static bool detectShift(QImage &img1, QImage &img2, int tileSize, int &dx, int &dy );

	static qint32 calculateFocusValue( QImage &cimg, int alg=4, QRect tile = QRect(), bool debugMode=false );
	static QList<QRect> findItems( QImage & imgIn, QImage tmplIn, int meth );

	
	static QList<QRect> findItems( cv::Mat &img, cv::Mat &tmpl, int meth );
	static QList<QPolygon> findEllipses( QImage & img, int minsize, int maxsize );
	static QList<QPolygon> getHuMoments( QImage & img, int minsize, int maxsize, double* hu );

	static QList<CellInfo> getCells( QImage & imgIn, int minsize, int maxsize );

	static QList<QPolygon> findEllipses1( QImage & imgIn, int minsize, int maxsize );
	static double findPeaks( QImage & img, int diamMin, int diamMax, int levelOf5=3, QList<QRect> * peaks=0 );
	static bool merge( QImage &imgout, const QImage &imgAdd );
	
	static int removeInhomogeniousAreas( QImage & img );
	static void normalize( QImage & img );
	static QMap<QByteArray,int> getSequenceHistogram( QImage & img, int seqLen, int levels );
	static int findLocMaxima( QImage & img, int step, int minTh, QList<QRect> * positions=0 );
	static int estimateTiter( double brightness, double exposure, double overexposure=0, double correctionFctor=1 );
	static QPair<double, double> getMeanAndDev ( const QList<double> );

	//Features
	static int featurePeaks( QImage & img, int diamMin, int diamMax, QList<QRect> *nuclei=0 );
 
	static void reduceBackgroundNoise( QImage &cimg, int );

    static void removeBackground( QImage &cimg, int );

    static void diffX( QImage & img, int n=5 );
	static void diffY( QImage & img, int n=5 );
	static void diffXY( QImage & img, int n=5 );

	static void overlayImageInfo( QImage &cimg, const QPoint & pos  );

};


#endif
#endif
