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

struct CLIB_EXPORT PDImageInfo
{
	static EImageContentType ExpectedImageType;
    unsigned long _histogram[256];
    int _bgLevel;
	int _bgLevel1;
    int _underexposureTh; //threshhold for underexposed values
    int _histogramMin;
    int _histogramMax;
    int _histogramWidth;
	int _meanPixValue; ///<average pixel value (for green channel)
    int _meanFgPixValue; ///<average pixel value excluding bg (for green channel)
    int _overexposure1; ///< ratio of overexposed on non bg bixels in 10tn of permil
	int _brightness; ///< 0-100 percentage of histogramm on the right side of 127
    int _bgRatio; ///< 0-100 percentage of pixels making up the backgroung
    QSize _imgSize;
	int _midCpl;
	double _exposure;
	double _maxExposure;
    PDImageInfo(){reset();}
	double value( const QString & varName )
	{
		if ( varName == "BL" )return _bgLevel;
		if ( varName == "BL1" )return  _bgLevel1;
		if ( varName == "UTH" )return  _underexposureTh; 
		if ( varName == "HMin" )return _histogramMin;
		if ( varName == "HMax" )return _histogramMax;
		if ( varName == "HW" )return _histogramWidth;
		if ( varName == "MV" )return _meanPixValue;
		if ( varName == "MFV" )return _meanFgPixValue;
		if ( varName == "OE" )return _overexposure1;
		if ( varName == "BR" )return _brightness;
		if ( varName == "BGR" )return _bgRatio;
		if ( varName == "W" ) return _imgSize.width();
		if ( varName == "H" ) return _imgSize.height();
		if ( varName == "CPL" ) return _midCpl;
		if ( varName == "E" ) return _exposure;
		if ( varName == "EMax" ) return _maxExposure;
		return 0; //TODO ERROR
	}
	QString toString(  )
	{
		QString out;
		out += QString("BL:%1 ").arg(_bgLevel);
		out += QString("BL1:%1 " ).arg(_bgLevel1);
		out += QString("UTH:%1 "  ).arg(_underexposureTh); 
		out += QString("HMin:%1 " ).arg(_histogramMin);
		out += QString("HMax:%1 " ).arg(_histogramMax);
		out += QString("HW:%1 "   ).arg(_histogramWidth);
		out += QString("MV:%1 "   ).arg(_meanPixValue);
		out += QString("MFV:%1 "  ).arg(_meanFgPixValue);
		out += QString("OE:%1 "   ).arg(_overexposure1);
		out += QString("BR:%1 "   ).arg(_brightness);
		out += QString("BGR:%1 "  ).arg(_bgRatio);
		out += QString("W:%1 "    ).arg(_imgSize.width());
		out += QString("H:%1 "    ).arg(_imgSize.height());
		out += QString("CPL:%1 "  ).arg(_midCpl);
		out += QString("E:%1 "    ).arg(_exposure);
		out += QString("EMax:%1"  ).arg(_maxExposure);
		return out;
	}
    void reset()
    {
        memset( _histogram, 0, sizeof(unsigned long) * 256 );
        _bgLevel=20;
		_bgLevel1=20;
        _underexposureTh = 0;
        _histogramMin=255;
        _histogramMax=0;
        _histogramWidth=0;
		_meanPixValue=0;
        _meanFgPixValue=0;
        _overexposure1=0;
		_brightness=0;
        _bgRatio=0;
        _imgSize=QSize(0,0);
		_midCpl=0;
		_exposure=0;
		_maxExposure=0;
    }
};


class CLIB_EXPORT CellInfo
{
public:
	QString _svmentry;
	QRect _rrect;
	//QImage _normalized;
};





class CLIB_EXPORT PDImageProcessing
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

	static bool preClassifyImage( QImage &cimg, int minHistWidth=30, int maxBgFloor=70, int maxBgRatio=70, int minCellsPerLine=3, int maxCellsPerLine=30, 
		PC_DCPSettings & nuclearSettings = PC_DCPSettings(),
		PreClassificationResults & result= PreClassificationResults::null, double eptCF=1.0 );

	static bool preClassifyAnca( QImage &cimg, int minHistWidth=30, int maxBgFloor=70, int maxBgRatio=70, int minCellsPerLine=3, int maxCellsPerLine=30, 
		PreClassificationResults & result= PreClassificationResults::null, double eptCF=1.0 );

	static bool preClassifyNdna( QImage &cimg, int minDnasPercent, int&nDnaPct, int cellMin=20, int cellMax=60, int dots=2, int minCells=30, double maxExposure=0.4, int peakMin = 4, int peakMax = 16 );

	static QList<QRect> findAncaes( QImage & img );
	static QList<QRect> findItems( QImage & imgIn, QImage tmplIn, int meth );

	
	static QList<QRect> findItems( cv::Mat &img, cv::Mat &tmpl, int meth );
	static QList<QPolygon> findEllipses( QImage & img, int minsize, int maxsize );
	static QList<QPolygon> getHuMoments( QImage & img, int minsize, int maxsize, double* hu );

	static QList<CellInfo> getCells( QImage & imgIn, int minsize, int maxsize );

	static QList<QPolygon> findEllipses1( QImage & imgIn, int minsize, int maxsize );
	static double findPeaks( QImage & img, int diamMin, int diamMax, int levelOf5=3, QList<QRect> * peaks=0 );
	static bool merge( QImage &imgout, const QImage &imgAdd );
    static bool findHEp2Nucleus( QImage & img );
	
	static bool findHEp2Nucleus1(  QImage & img, int & level, int diamMin=40, int diamMax=100, QList<QRect> * nuclei=0 );
	static int removeInhomogeniousAreas( QImage & img );
	static void normalize( QImage & img );
	static QMap<QByteArray,int> getSequenceHistogram( QImage & img, int seqLen, int levels );
	static int findLocMaxima( QImage & img, int step, int minTh, QList<QRect> * positions=0 );
	static int estimateTiter( double brightness, double exposure, double overexposure=0, double correctionFctor=1 );
	static QPair<double, double> getMeanAndDev ( const QList<double> );

	//Features
	static int featurePeaks( QImage & img, int diamMin, int diamMax, QList<QRect> *nuclei=0 );

    static bool findHEp2Nucleus( QImage & img, int & level, int diamMin=40, int diamMax=100, QList<QRect> * nuclei=0  );

	static int getCytoplasmaIndex(QImage & imgIn);

	static bool classifyImage( QImage &cimg, int=-1 ); //simple pattern recognition test

    /*! Creates green histogram
    
    */
    static void getImageInfo( QImage &cimg, PDImageInfo & imgInfo ); 
	static void getImageInfoANCA( QImage &cimg, PDImageInfo & imgInfo ); //Q&D split betw. ANCA and ANY. Because ANCAs are not OK and ANAs mastn't be touched!

	static void reduceBackgroundNoise( QImage &cimg, int );

    static void removeBackground( QImage &cimg, int );

    static void diffX( QImage & img, int n=5 );
	static void diffY( QImage & img, int n=5 );
	static void diffXY( QImage & img, int n=5 );

	static void overlayImageInfo( QImage &cimg, const QPoint & pos  );

};


#endif
#endif
