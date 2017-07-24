#include "testing.h"
#include <QImage>
#include <pdimageprocessing.h>
#include <cells.h>
#include <vector>
#include <QtCore/qmath.h>
#include <QElapsedTimer>

//#include <cv.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace cv;


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

QMutex CeMx;

int test_main()
{
    // Data for visual representation
    int width = 512, height = 512;
    Mat image = Mat::zeros(height, width, CV_8UC3);

    // Set up training data
    float labels[4] = {1.0, -1.0, -1.0, -1.0};
    Mat labelsMat(4, 1, CV_32FC1, labels);

    float trainingData[4][2] = { {501, 10}, {255, 10}, {501, 255}, {10, 501} };
    Mat trainingDataMat(4, 2, CV_32FC1, trainingData);

    // Set up SVM's parameters
    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
	params.kernel_type = CvSVM::NU_SVC;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

    // Train the SVM
    CvSVM SVM;
    SVM.train(trainingDataMat, labelsMat, Mat(), Mat(), params);

    Vec3b green(0,255,0), blue (255,0,0);
    // Show the decision regions given by the SVM
    for (int i = 0; i < image.rows; ++i)
        for (int j = 0; j < image.cols; ++j)
        {
            Mat sampleMat = (Mat_<float>(1,2) << j,i);
            float response = SVM.predict(sampleMat);

            if (response == 1)
                image.at<Vec3b>(i,j)  = green;
            else if (response == -1)
                 image.at<Vec3b>(i,j)  = blue;
        }

    // Show the training data
    int thickness = -1;
    int lineType = 8;
    circle( image, Point(501,  10), 5, Scalar(  0,   0,   0), thickness, lineType);
    circle( image, Point(255,  10), 5, Scalar(255, 255, 255), thickness, lineType);
    circle( image, Point(501, 255), 5, Scalar(255, 255, 255), thickness, lineType);
    circle( image, Point( 10, 501), 5, Scalar(255, 255, 255), thickness, lineType);

    // Show support vectors
    thickness = 2;
    lineType  = 8;
    int c     = SVM.get_support_vector_count();

    for (int i = 0; i < c; ++i)
    {
        const float* v = SVM.get_support_vector(i);
        circle( image,  Point( (int) v[0], (int) v[1]),   6,  Scalar(128, 128, 128), thickness, lineType);
    }

    imwrite("result.png", image);        // save the image

    imshow("SVM Simple Example", image); // show it to the user
    waitKey(0);
	exit(0);
	return 0;
}

//const int runtest = test_main();

QString LocalMaxInfo ="Finds local maximums";
REGISTER_FEATURE( LocalMax, FeatureLocalMax, LocalMaxInfo );

FeatureLocalMax::FeatureLocalMax()
{
	_paramInfos.append( ParamInfo( "step", "Minimal distance between peaks"  ) );
	_paramInfos.append( ParamInfo( "minThreshold" ) );
}

FeatureLocalMax::~FeatureLocalMax(){}

QString FeatureLocalMax::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureLocalMax::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	if (!params.contains("step") || !params.contains("minThreshold"))
		return res;

	QElapsedTimer timer;
	timer.start();
	res->_value = ImageProcessing::findLocMaxima( img, params["step"].toInt(), params["minThreshold"].toInt(), &positions );
	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;	
}


QString ImgParamInfo ="Image parameter value. [BL,BL1,UTH,HMin,HMax,HW,MV,MFV,OE,BR,BGR,W,H,CPL,E,EMax]";
REGISTER_FEATURE( ImgParam, FeatureImgParam, ImgParamInfo );

FeatureImgParam::FeatureImgParam()
{
	QString constr("[BL,BL1,UTH,HMin,HMax,HW,MV,MFV,OE,BR,BGR,W,H,CPL,E,EMax]" );
	_paramInfos.append( ParamInfo( "param", QString("Parameter extracted from image.\nDefined params:%1").arg(constr), constr ) );
}

FeatureImgParam::~FeatureImgParam(){}

QString FeatureImgParam::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureImgParam::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	if ( !params.contains("param") )
		return res;

	QElapsedTimer timer;
	timer.start();

	PDImageInfo imginf; //todo optimaze - cache
	PDImageProcessing::getImageInfo( img, imginf );

	QString pn = params["param"].toString();

	res->_value = imginf.value( pn );

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}

//////////////////////////

QString PeaksInfo ="Intensity peaks between diameter min and max";
REGISTER_FEATURE( Peaks, FeaturePeaks, PeaksInfo );

FeaturePeaks::FeaturePeaks()
{
	ParamInfo pi( "min", "min diameter", "1-max" );
	_paramInfos.append(pi);

	pi._name = "max" ;
	pi._info = "max diameter", "min-200" ;
	_paramInfos.append(pi);
}

FeaturePeaks::~FeaturePeaks(){}

QString FeaturePeaks::version()
{
	return "0.0.1";
}

TPFeatureResults FeaturePeaks::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int min, max;
	if (!params.contains("min"))
	{
		//todo error
		return res;
	}
	if (!params.contains("max"))
	{
		//todo error
		return res;
	}

	min = params["min"].toInt();
	max = params["max"].toInt();
	QElapsedTimer timer;
	timer.start();

	res->_value = ImageProcessing::findPeaks( img, min, max, 3, &positions );

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


//////////////////////////

QString PeaksPerLineInfo ="Intensity peaks within a horizontal line between min and max and above a threshold";
REGISTER_FEATURE( PeaksPerLine, FeaturePeaksPerLine, PeaksPerLineInfo );

FeaturePeaksPerLine::FeaturePeaksPerLine()
{
	ParamInfo pi( "minlen" );
	_paramInfos.append(pi);

	pi._name = "maxlen" ;
	_paramInfos.append(pi);

	pi._name = "th" ;
	_paramInfos.append(pi);
}

FeaturePeaksPerLine::~FeaturePeaksPerLine(){}

QString FeaturePeaksPerLine::version()
{
	return "0.0.2";
}

TPFeatureResults FeaturePeaksPerLine::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int min, max, th, dummy;
	if (!params.contains("minlen"))
	{
		//todo error
		return res;
	}
	if (!params.contains("maxlen"))
	{
		//todo error
		return res;
	}

	if (!params.contains("th"))
	{
		//todo error
		return res;
	}

	QElapsedTimer timer;
	timer.start();

	min = params["minlen"].toInt();
	max = params["maxlen"].toInt();
	th = params["th"].toInt();
	int mean=0;
	double peaks=0; 

	enum PeakState{ Start, Up, Down };
	PeakState s=Start;


	for ( int y=0, yend=img.size().height(); y<yend;++y )
	{
		int peakLen=0;

		for ( int x=0, xend=img.size().width(); x<xend;++x )
		{
			unsigned char pix = img.pixel(x,y) >> 8;
			switch( s )
			{
			case Start:
				if ( pix > th )
					s = Up;
				break;
			case Up:
				if ( pix > th )
				{
					++peakLen;
				}
				else if ( pix <= th )
					s = Down;
				break;
			case Down:
				if ( peakLen<max && peakLen>min )
				{
					++peaks;
					positions.append( QRect( x-peakLen, y, peakLen, 1 ) );
				}
				peakLen=0;
				s = Start;

				break;
			}			
		}		

	}
	peaks /= img.size().height();

	res->_value = peaks;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


//////////////////////////

QString BlobsInfo ="Blobs having diameter min and max, hight above th of max levels lmax and eroded ";
REGISTER_FEATURE( Blobs, FeatureBlobs, BlobsInfo );

FeatureBlobs::FeatureBlobs()
{
	ParamInfo pi( "min" );
	_paramInfos.append(pi);

	pi._name = "max" ;
	_paramInfos.append(pi);

	pi._name = "th" ;
	_paramInfos.append(pi);

	pi._name = "lmax" ;
	_paramInfos.append(pi);

	pi._name = "erosion" ;
	_paramInfos.append(pi);
}

FeatureBlobs::~FeatureBlobs(){}

QString FeatureBlobs::version()
{
	return "0.0.1";
}


void FindBlobs(const cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs)
{
    blobs.clear();

    // Fill the label_image with the blobs
    // 0  - background
    // 1  - unlabelled foreground
    // 2+ - labelled foreground

    cv::Mat label_image;
    binary.convertTo(label_image, CV_32SC1);

    int label_count = 2; // starts at 2 because 0,1 are used already

    for(int y=0; y < label_image.rows; y++) {
        int *row = (int*)label_image.ptr(y);
        for(int x=0; x < label_image.cols; x++) {
            if(row[x] != 1) {
                continue;
            }

            cv::Rect rect;
            cv::floodFill(label_image, cv::Point(x,y), label_count, &rect, 0, 0, 4);

            std::vector <cv::Point2i> blob;

            for(int i=rect.y; i < (rect.y+rect.height); i++) {
                int *row2 = (int*)label_image.ptr(i);
                for(int j=rect.x; j < (rect.x+rect.width); j++) {
                    if(row2[j] != label_count) {
                        continue;
                    }

                    blob.push_back(cv::Point2i(j,i));
                }
            }

            blobs.push_back(blob);

            label_count++;
        }
    }
}

TPFeatureResults FeatureBlobs::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int min, max, th, lmax, erosion;
	if (!params.contains("min"))
	{
		//todo error
		return res;
	}
	if (!params.contains("max"))
	{
		//todo error
		return res;
	}

	if (!params.contains("th"))
	{
		//todo error
		return res;
	}

	if (!params.contains("lmax"))
	{
		//todo error
		return res;
	}

	if (!params.contains("erosion"))
	{
		//todo error
		return res;
	}

	int pyrFactor = 4;

	QElapsedTimer timer;
	timer.start();

	min = params["min"].toInt() / pyrFactor;
	max = params["max"].toInt() / pyrFactor;
	th = params["th"].toInt();
	lmax = params["lmax"].toInt();
	erosion = params["erosion"].toInt();

	

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
	cv::Mat m(_T::toMat(small));

	//imshow("Small", m );
	
	//cv::Mat norm = cv::Mat::zeros( m.rows, m.cols, CV_8UC1 );		

	cv::normalize( m,m, 0, lmax, CV_MINMAX );
	//imshow("Normalized", m );

	cv::threshold(m ,m,th, 255,  CV_THRESH_BINARY);

	Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                      cv::Size(3, 3), 
                      cv::Point(1, 1) );

	if (erosion > 0)
	{
		cv::erode(m,m,kernel);
		cv::dilate(m,m,kernel);
	}

	//imshow("Normalized", m );
				
	std::vector<std::vector<cv::Point> > contours;		

	cv::findContours(m, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int foundBlobs=0;
	for( unsigned int i = 0; i < contours.size(); i++ )
	{ 
		if( contours[i].size() < 3  )
			continue;

		double area = cv::contourArea(Mat(contours[i]) );
		if ( (area > min * min) && ( area < max * max ) )
		{
			foundBlobs++;
			Moments mu = moments( contours[i], false );
			positions.append( QRect( (mu.m10/mu.m00)*pyrFactor, (mu.m01/mu.m00)*pyrFactor, min*pyrFactor, min*pyrFactor) );
		}
	}

	res->_value = foundBlobs;
	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


// END BLOBS////////////////////////


QString nDNAIndexInfo ="Probability to be a positive nDNA. nDots within a circle of diameter min-max";
REGISTER_FEATURE( nDNAIndex, FeatureNdnaIndex, nDNAIndexInfo );

FeatureNdnaIndex::FeatureNdnaIndex()
{
	ParamInfo pi( "cellMin" );
	_paramInfos.append(pi);

	ParamInfo pi1( "cellMax" );
	_paramInfos.append(pi1);

	ParamInfo pi2( "dots" );
	_paramInfos.append(pi2);
}

FeatureNdnaIndex::~FeatureNdnaIndex(){}

QString FeatureNdnaIndex::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureNdnaIndex::extract( QImage & img, TFeatParams params )
{	
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	if (!params.contains("cellMin"))
	{
		//todo error
		return res;
	}
	if (!params.contains("cellMax"))
	{
		//todo error
		return res;
	}
	if (!params.contains("dots"))
	{
		//todo error
		return res;
	}

	QElapsedTimer timer;
	timer.start();

	int min = params["cellMin"].toInt();
	int max = params["cellMax"].toInt();
	int dots = params["dots"].toInt();
	int peakMin = 4;
	int peakMax = 16;

	QList<QPolygon> pol =  PDImageProcessing::findEllipses( img, min, max );	
	QList<QRect> cells;	

	foreach( QPolygon p, pol )
	{
		cells.append( p.boundingRect() );
	}

	if ( cells.size() < 10 )
	{
		res->_value = 0;
		return res;
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
	
	int cnt=0;
	foreach (QString k, posNdnas.keys() )
	{
		nDnaPosCandidate & nc = posNdnas[ k ];
		if( nc._peaks.size() >= dots )
		{
			positions.append( nc._rect );
			++cnt;
		}
	}

	res->_value = cnt;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


//////////////////////////
QString NumOfStroksInfo ="Lines having similar intensity and defined length";
REGISTER_FEATURE( NumOfStroks, FeatureNumOfStroks,NumOfStroksInfo );

FeatureNumOfStroks::FeatureNumOfStroks()
{
	ParamInfo th1( "thresholdMin" );
	_paramInfos.append(th1);

	ParamInfo th2( "thresholdMax" );
	_paramInfos.append(th2);

	ParamInfo lmin( "lenMin" );
	_paramInfos.append(lmin);

	ParamInfo lmax( "lenMax" );
	_paramInfos.append(lmax);

}

FeatureNumOfStroks::~FeatureNumOfStroks(){}

QString FeatureNumOfStroks::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureNumOfStroks::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int lmin, lmax, thmin, thmax;
	if (!params.contains("thresholdMin"))
	{
		//todo error
		return res;
	}
	if (!params.contains("thresholdMax"))
	{
		//todo error
		return res;
	}

	if (!params.contains("lenMin"))
	{
		//todo error
		return res;
	}
	if (!params.contains("lenMax"))
	{
		//todo error
		return res;
	}

	QElapsedTimer timer;
	timer.start();

	lmin = params["lenMin"].toInt();
	lmax = params["lenMax"].toInt();
	thmin = params["thresholdMin"].toInt();
	thmax = params["thresholdMax"].toInt();

	int cnt=0;
	int len=0;

	enum { Searching, Started, Overlength } state = Searching;
	int lines = img.height();
	int pixPerLine = img.width();
	unsigned char gp=0;
	for (int l=0; l< lines; ++l)
	{
		QRgb* pix = (QRgb*)img.scanLine(l);
		for (int p=0; p<pixPerLine; p++)
		{
			gp = qGreen(*pix);
			if( state == Overlength )
			{
				if ( gp < thmin )
				{
					len=0;
					state = Searching;
				}
				else
					len++;
			}
			else if( state == Searching )
			{
				if ( gp >= thmin && gp <= thmax )
				{
					len++;
					state = Started;
				}
				else
					len=0;
			}
			else if( state == Started )
			{
				if ( gp >= thmin && gp <= thmax && len < lmax )
				{
					len++;
				}
				else if ( gp >= thmin && len >= lmax )
				{
					state = Overlength;
				}
				else if ( gp < thmin && len >= lmin )
				{
					cnt++;
					state = Searching;
					len=0;

				}				
			}
			pix++;
		}		
	}			

	res->_value = cnt;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


//////////////////////////
QString SpinsPerLineInfo ="Spins per line with defined threshold";
REGISTER_FEATURE( SpinsPerLine, FeatureSpinsPerLine, SpinsPerLineInfo );

FeatureSpinsPerLine::FeatureSpinsPerLine()
{
	ParamInfo th( "threshold" );
	_paramInfos.append(th);
}

FeatureSpinsPerLine::~FeatureSpinsPerLine(){}

QString FeatureSpinsPerLine::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureSpinsPerLine::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	if (!params.contains("threshold"))
	{
		//todo error
		return res;
	}

	int th = params["threshold"].toInt();

	QElapsedTimer timer;
	timer.start();

	double cnt=0;
	int v1=0;
	int v2=0;
	int v3=0;

	enum { Searching, Started, Overlength } state = Searching;
	int lines = img.height();
	if (lines  == 0)
		return res;

	int pixPerLine = img.width();
	
	for (int l=0; l< lines; ++l)
	{
		QRgb* pix = (QRgb*)img.scanLine(l);
		for (int p=0; p<pixPerLine; p++)
		{
			v1=v2;
			v2=v3;
			v3 = qGreen(*pix);
			pix++;
			if ( (v1 < th && v2 < th ) || (v2 < th && v3 < th ) )
				continue;
			if ( (v1 < v2 && v3 < v2)  //v2 is peak
				|| (v1 > v2 && v3 > v2) ) //v2 is low
			{
				cnt+=1;
				positions.append( QRect( p, l, 5, 3 ) );
			}

			
		}		
	}			

	res->_value = cnt / lines;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


//////////////////////////
QString MatchHistogramInfo ="Compares histograms of several cell types using different methods";
REGISTER_FEATURE( MatchHistogram, FeatureMatchHistogram, MatchHistogramInfo );

FeatureMatchHistogram::FeatureMatchHistogram()
{
	ParamInfo ct( "cell", "Cell type", "[1,2]" );
	_paramInfos.append(ct);

	ParamInfo m( "meth", "Method (s. OpenCV)", "[0,1,2,3]"  );
	_paramInfos.append(m);
}

FeatureMatchHistogram::~FeatureMatchHistogram(){}

QString FeatureMatchHistogram::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureMatchHistogram::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;
	if (!params.contains("cell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("meth"))
	{
		//todo error
		return res;
	}

	int ct = params["cell"].toInt();

	QElapsedTimer timer;
	timer.start();

	PDImageInfo imginf; //todo optimaze - cache
	PDImageProcessing::getImageInfo( img, imginf );

	if ( ct == 1 ) // Centromere
	{
		//TODO load appropriate image using k-nearest nh
		//if ( imginf._brightness > 100 )
			testImg = QImage(":/res/cells/centr_br137.png");
		//else
		//	testImg = QImage(":/res/cells/centr_br77.png");
	}
	else if ( ct == 2 ) //corseSpec
	{
		//TODO load appropriate image using k-nearest nh
		//if ( imginf._brightness > 150 )
			testImg = QImage(":/res/cells/corspec_br194.png");
		//else
		//	testImg = QImage(":/res/cells/corspec_br144.png");
	}
	
	int meth = params["meth"].toInt();

	QList<QPolygon> pol =  PDImageProcessing::findEllipses( img, 40, 150 );
	if ( pol.size() == 0 )
		return res;

	double hmean=0.0;

	foreach( QPolygon p, pol )
	{
			
		hmean +=  PDImageProcessing::compareHistogramm(img, testImg, meth, p.boundingRect() );
		positions.append( p.boundingRect());
	}
	hmean /=pol.size();

	res->_value = hmean;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}



//////////////////////////

QString SequenceInfo ="Number of intensity sequences (line-wize) in 3x pyrDown image having defined number of intensities";
REGISTER_FEATURE( Sequence, FeatureSequence, SequenceInfo );

FeatureSequence::FeatureSequence()
{

	ParamInfo pi;
	pi._name= "levels";
	_paramInfos.append(pi);

	pi._name= "seq";
	_paramInfos.append(pi);
}

FeatureSequence::~FeatureSequence(){}

QString FeatureSequence::version()
{
	return "0.0.2";
}

TPFeatureResults FeatureSequence::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;

	if (!params.contains("levels"))
	{
		//todo error
		return res;
	}

	if (!params.contains("seq"))
	{
		//todo error
		return res;
	}

	QElapsedTimer timer;
	timer.start();
	
	int levels = params["levels"].toInt();
	QByteArray seq = params["seq"].toString().toAscii();
	for (int i=0; i<seq.length(); ++i)
		seq[i] = seq[i] - '0'; //Ascii to int

	int seqLen = seq.length();

	QMap<QByteArray,int> sh = PDImageProcessing::getSequenceHistogram( img, seqLen, levels );

	if ( sh.contains( seq ) )
		res->_value = sh[ seq ]; 
	else
		res->_value = 0;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}

//////////////////////////
QString HuMeanInfo ="Hu-Moments of cells with defined size";
REGISTER_FEATURE( HuMean, FeatureHuMean, HuMeanInfo );

FeatureHuMean::FeatureHuMean()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

	pi._name= "huNum";
	pi._constrains = "[0,1,2,3,4,5,6]";
	_paramInfos.append(pi);
}

FeatureHuMean::~FeatureHuMean(){}

QString FeatureHuMean::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureHuMean::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;
	if (!params.contains("minCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("maxCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("huNum"))
	{
		//todo error
		return res;
	}
	
	QElapsedTimer timer;
	timer.start();

	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();
	int hunum = params["huNum"].toInt();

	if (hunum>=0 && hunum <= 6)
	{

		double hu[7]={0.0};

		int pyrFactor = 4; //TOODO optimize,get scaled image from cache

		QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
		cv::Mat m(_T::toMat(small));

		CellExtractor ce;
		ce.setMinCell(min/pyrFactor);
		ce.setMaxCell(max/pyrFactor);
		CeMx.lock();
		TRotatedRectList cells = ce.findCellRects( m );
		CeMx.unlock();
		foreach( cv::RotatedRect rr,  cells )
		{
			cv::Rect cr = rr.boundingRect();
			positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
		}

		int cnt=0;
		foreach( cv::RotatedRect rr,  cells )
		{
			int x = rr.center.x-rr.size.width/2;
			int y = rr.center.y-rr.size.height/2;
			int w = rr.size.width;
			int h = rr.size.height;
			cv::Rect roirect(
					cv::Point(x,y ), 
					cv::Size( w,h ) 
					);


			if(x>0 && y>0 && x+w < m.cols && y+h < m.rows)
			{
				double hutmp[7]={0.0};
				cv::Mat cell = m(roirect);				
				cv::Moments m =  cv::moments(cell);			
				cv::HuMoments(m, hutmp );
				++cnt;
				for (int i=0; i<7; ++i)
					hu[i] += hutmp[i];
			}
		}

		if (cnt>0)
		{
			for (int i=0; i<7; ++i)
				hu[i] = hu[i] / cnt;
		}
				
		res->_value = hu[hunum];
	}

	else if (hunum>=10 && hunum <= 16)
	{

		double hu[7]={0.0};

		int pyrFactor = 4; //TOODO optimize,get scaled image from cache

		QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
		cv::Mat m(_T::toMat(small));

		CellExtractor ce;
		ce.setMinCell(min/pyrFactor);
		ce.setMaxCell(max/pyrFactor);
		CeMx.lock();
		TRotatedRectList cells = ce.findCellRects( m );

		foreach( cv::RotatedRect rr,  cells )
		{
			cv::Rect cr = rr.boundingRect();
			positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
		}
		
		std::vector< std::vector<cv::Point> > cont = ce.cellContours();
		CeMx.unlock();

		int cnt=0;
		foreach( std::vector<cv::Point> c, cont )
		{
			
				double hutmp[7]={0.0};
		
				cv::Moments m =  cv::moments(c);			
				cv::HuMoments(m, hutmp );
				++cnt;
				for (int i=0; i<7; ++i)
					hu[i] += hutmp[i];
		}

		if (cnt>0)
		{
			for (int i=0; i<7; ++i)
				hu[i] = hu[i] / cnt;
		}
				
		res->_value = hu[hunum-10];
	}

	else if (hunum>=20 && hunum <= 39)
	{

		double hu[20]={0.0};

		int pyrFactor = 4; //TOODO optimize,get scaled image from cache

		QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
		cv::Mat m(_T::toMat(small));

		CellExtractor ce;
		ce.setMinCell(min/pyrFactor);
		ce.setMaxCell(max/pyrFactor);
		CeMx.lock();
		TRotatedRectList cells = ce.findCellRects( m );
		std::vector< std::vector<cv::Point> > cont =  ce.cellContours() ;
		CeMx.unlock();
		foreach( cv::RotatedRect rr,  cells )
		{
			cv::Rect cr = rr.boundingRect();
			positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
		}

		int cnt=0;
		foreach( std::vector<cv::Point> c, cont )
		{
			
				double hutmp[20]={0.0};
		
				cv::Moments m =  cv::moments(c);			
				hu[0] += m.m00;
				hu[1] += m.m01;
				hu[2] += m.m02;
				hu[3] += m.m03;
				hu[4] += m.m10;
				hu[5] += m.m11;
				hu[6] += m.m12;
				hu[7] += m.m20;
				hu[8] += m.m21;
				hu[9] += m.m30;
				hu[10] += m.mu02;
				hu[11] += m.mu03;
				hu[12] += m.mu11;
				hu[13] += m.mu12;
				hu[14] += m.mu20;
				hu[15] += m.mu21;
				hu[16] += m.mu30;
				hu[17] += m.nu02;
				hu[18] += m.nu03;
				hu[19] += m.nu11;
				cnt++;
		}

		if (cnt>0)
		{
			for (int i=0; i<20; ++i)
				hu[i] = hu[i] / cnt;
		}
				
		res->_value = hu[hunum-20];
	}

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}



//////////////////////////
QString CellMetricsInfo ="Cell Metrics. thMin/Max: 0-5 Metrics:0 - num of cells\n1-Area, 2-mean of cell-centers, 3/4-convexity defects, 5-BRISK, 6-BLOBS,\n"
"7-FAST, 8-GFtT, 9-Mser, 10-STAR, 11-Dense 12-Circles 5-15; op: n-none,SX[Y]-Sobel, B-Blur, L-Laplacian; param: n-none, size, response";
REGISTER_FEATURE( CellMetrics, FeatureCellMetrics, CellMetricsInfo );

FeatureCellMetrics::FeatureCellMetrics()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

	pi._name= "roi";
	_paramInfos.append(pi);

	pi._name= "similar";
	_paramInfos.append(pi);

	pi._name= "minNum";
	_paramInfos.append(pi);

	pi._name= "metric";
	_paramInfos.append(pi);

    pi._name= "param1";
	_paramInfos.append(pi);

	pi._name= "op";
	_paramInfos.append(pi);
}

FeatureCellMetrics::~FeatureCellMetrics(){}

QString FeatureCellMetrics::version()
{
	return "0.0.2";
}

TPFeatureResults FeatureCellMetrics::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int minNum=30;
	int roi=0;
	int similar=0;



	QImage testImg;
	if (!params.contains("minCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("maxCell"))
	{
		//todo error
		return res;
	}

	if (params.contains("roi"))
	{
		roi = params["roi"].toInt();
	}

	if (params.contains("similar"))
	{
		similar = params["similar"].toInt();
	}

	if (params.contains("minNum"))
	{
		minNum = params["minNum"].toInt();
	}

	if (!params.contains("metric"))
	{
		//todo error
		return res;
	}

	QElapsedTimer timer;
	timer.start();

	QString param1=""; //default
	if (params.contains("param1"))
	{
		param1 = params["param1"].toString();
	}
	
	QString op=""; //default
	if (params.contains("op"))
	{
		op = params["op"].toString();
	}

	int pyrFactor = 4; //TOODO optimize,get scaled image from cache

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
	cv::Mat m(_T::toMat(small));

	CellExtractor ce;
	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();
	int metric = params["metric"].toInt();
	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);

	CeMx.lock();
	TRotatedRectList cells = ce.findCellRects( m, minNum, similar, roi );

	std::vector< std::vector<cv::Point> > cont =  ce.cellContours() ;
	CeMx.unlock();

	if(op.contains("B"))
	{
		GaussianBlur( m, m, Size(3,3), 0, 0, BORDER_DEFAULT );
		//imshow("BLUR", m );
	}
	if(op.contains("SXY"))
	{
		Mat grad;

		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;

		Sobel( m, grad_x, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT );
		convertScaleAbs( grad_x, abs_grad_x );

		Sobel( m, grad_y, CV_8U, 0, 1, 3, 1, 0, BORDER_DEFAULT );
		convertScaleAbs( grad_y, abs_grad_y );

		addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, m );

		//imshow( "SOBEL", m );
	}
	else if(op.contains("SX"))
	{
		Sobel( m, m, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT );
		//imshow( "SOBEL", m );
	}
	else if(op.contains("SY"))
	{
		Sobel( m, m, CV_8U, 0, 1, 3, 1, 0, BORDER_DEFAULT );
		//imshow( "SOBEL", m );
	}
	if(op.contains("L"))
	{
		Laplacian( m, m, CV_8U, 3, 1, 0, BORDER_DEFAULT );
		//imshow( "LAPLACE", m );
	}

	ce.setCellsImage( m );

	foreach( cv::RotatedRect rr,  cells )
	{
		cv::Rect cr = rr.boundingRect();
		positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
	}

	double value=0;
	if ( metric == 0 ) //number of cells
	{
		value = cells.size();
	}
	if ( metric == 1 ) //area TODO: use any types
	{
		double area=0;
		foreach( cv::RotatedRect rr,  cells )
		{
			area += rr.size.width * rr.size.height;
		}
		area *= pyrFactor*pyrFactor;

		if ( cells.size()>0 )
			value = area / cells.size();
	}

	if ( metric == 2 ) //mean of cell-centers / mean of image  area TODO: use any types
	{
		cv::Scalar entMean = mean(m);
		cv::Scalar centersMean;
		foreach( cv::RotatedRect rr,  cells )
		{
			int x = rr.center.x-rr.size.width/4;
			int y = rr.center.y-rr.size.height/4;
			int w = rr.size.width/2;
			int h = rr.size.height/2;
			cv::Rect roirect(
					cv::Point(x,y ), 
					cv::Size( w,h ) 
					);


			if(x>0 && y>0 && x+w < m.cols && y+h < m.rows)
			{
				cv::Mat roi = m(roirect);				
				centersMean += cv::mean(roi) / cv::mean(m);
			}
		}
		
		if ( entMean[0] > 0 && cells.size()>0 )
			value = ( centersMean[0]/cells.size() ) / entMean[0];
	}

	if ( metric == 3 || metric == 4 ) //convexity defects TODO: use name instead of number for param
	{
		try{
			cv::vector<int> convexHull_IntIdx;
			int DefectCnt=0;
			for( std::vector< std::vector<cv::Point> >::const_iterator cit=cont.begin(), end=cont.end(); cit!=end; cit++ )
			{
				cv::convexHull(*cit, convexHull_IntIdx, true);
				if ( convexHull_IntIdx.size() > 0 )
				{
					std::vector<cv::Vec4i> convexityDefectsSet;   
					
					cv::convexityDefects( *cit, convexHull_IntIdx, convexityDefectsSet);
					for( std::vector<cv::Vec4i>::const_iterator dit=convexityDefectsSet.begin(), dend=convexityDefectsSet.end(); dit!=dend; dit++ )
					{
						value += (*dit)[3];
						DefectCnt++;
					}
				}
			}

			if ( DefectCnt>0 )
				value /= DefectCnt;		
		}
		catch(...)
		{
			return res;
		}
	}	

	if (metric == 5) //BRISK
	{
		cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create("BRISK");


		std::vector<cv::KeyPoint> keypoints_1;

		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			//TODO detector->detect( img_1, keypoints_1, ce.getM );
			if (keypoints_1.size() < 1)
				continue;
			
			value += keypoints_1.size();
			 
		}
		if ( cells.size()>0 )
			 value /= cells.size();
		else 
			value = -1;
	}

	if (metric == 6) //BLOBS
	{
/*
1	0.315224	0.268054
2	0.850956	0.263327
3	0.162662	0.112087
4	0.29146	0.165396
*/
		cv::SimpleBlobDetector::Params params;
		params.minDistBetweenBlobs = 16.0f / pyrFactor;
		params.filterByInertia = false;
		params.filterByConvexity = false;
		params.filterByColor = false;
		params.filterByCircularity = false;
		params.filterByArea = true;
		params.minArea = 16.0f / pyrFactor;
		params.maxArea = 250000.0f / pyrFactor;


		cv::SimpleBlobDetector detector(params);

		std::vector<cv::KeyPoint> keypoints_1;

		positions.clear();


		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			detector.detect( img_1, keypoints_1 );
			value += keypoints_1.size();

			if( keypoints_1.size() )
			{				
				cv::Rect cr = rr.boundingRect();
				positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));

				foreach( cv::KeyPoint kp, keypoints_1 )
					positions.append(QRect(cr.x*pyrFactor + kp.pt.x * pyrFactor, cr.y*pyrFactor + kp.pt.y * pyrFactor, 10, 10 ));
			}

		}
		if ( cells.size()>0 )
			value /= cells.size();
	}

	if (metric == 7) //FAST
	{
/*
1	8.70069	3.88124
2	13.4668	2.37913
3	7.36793	2.94347
4	3.70777	0.576473
*/

		cv::FastFeatureDetector detector;

		std::vector<cv::KeyPoint> keypoints_1;

		positions.clear();

		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			detector.detect( img_1, keypoints_1 );
			value += keypoints_1.size();

			if( keypoints_1.size() )
			{				
				cv::Rect cr = rr.boundingRect();
				positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));

				foreach( cv::KeyPoint kp, keypoints_1 )
					positions.append(QRect(cr.x*pyrFactor + kp.pt.x * pyrFactor, cr.y*pyrFactor + kp.pt.y * pyrFactor, 10, 10 ));
			}

		}
		if ( cells.size()>0 )
			value /= cells.size();
	}

	if (metric == 8) //GFtT
	{
/*
1	8.89095	1.80954
2	12.408	1.30406
3	13.0061	2.47924
4	4.77437	0.646114
*/

		cv::GoodFeaturesToTrackDetector detector;

		std::vector<cv::KeyPoint> keypoints_1;

		positions.clear();

		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			detector.detect( img_1, keypoints_1 );

			if (keypoints_1.size() < 1)
				continue;
			
			if ( param1 == "size" )
			{
				float km=0;
				foreach( cv::KeyPoint k,  keypoints_1 )
					km +=k.size;
				km/=keypoints_1.size();			
				value += km/keypoints_1.size();			
			}
			else if ( param1 == "response" )
			{
				float rm=0;
				foreach( cv::KeyPoint k,  keypoints_1 )
					rm +=k.response;
				value += rm/keypoints_1.size();			
			}
			else
				value += keypoints_1.size();

			value += keypoints_1.size();

			if( keypoints_1.size() )
			{				
				cv::Rect cr = rr.boundingRect();
				positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));

				foreach( cv::KeyPoint kp, keypoints_1 )
					positions.append(QRect(cr.x*pyrFactor + kp.pt.x * pyrFactor, cr.y*pyrFactor + kp.pt.y * pyrFactor, 10, 10 ));
			}

		}
		if ( cells.size()>0 )
			value /= cells.size();	
	}

	
	if (metric == 9) //Mser
	{
/*
1	3.33222	1.74913
2	8.1298	1.45897
3	5.33091	1.52839
4	2.80519	0.70107
*/

		cv::MserFeatureDetector detector;

		std::vector<cv::KeyPoint> keypoints_1;

		positions.clear();

		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			detector.detect( img_1, keypoints_1 );

			if (keypoints_1.size() < 1)
				continue;
			
			if ( param1 == "size" )
			{
				float km=0;
				foreach( cv::KeyPoint k,  keypoints_1 )
					km +=k.size;
				km/=keypoints_1.size();			
				value += km/keypoints_1.size();			
			}
			else if ( param1 == "response" )
			{
				float rm=0;
				foreach( cv::KeyPoint k,  keypoints_1 )
					rm +=k.response;
				value += rm/keypoints_1.size();			
			}
			else
				value += keypoints_1.size();

			value += keypoints_1.size();

			if( keypoints_1.size() )
			{				
				cv::Rect cr = rr.boundingRect();
				positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));

				foreach( cv::KeyPoint kp, keypoints_1 )
					positions.append(QRect(cr.x*pyrFactor + kp.pt.x * pyrFactor, cr.y*pyrFactor + kp.pt.y * pyrFactor, 10, 10 ));
			}

		}
		if ( cells.size()>0 )
			value /= cells.size();	
	}
	
	if (metric == 10) //STAR 
	{
/*
1	0	0
2	0	0
3	0	0
4	0	0
*/

		cv::StarFeatureDetector detector;

		std::vector<cv::KeyPoint> keypoints_1;

		positions.clear();

		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			detector.detect( img_1, keypoints_1 );
			value += keypoints_1.size();

			if( keypoints_1.size() )
			{				
				cv::Rect cr = rr.boundingRect();
				positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));

				foreach( cv::KeyPoint kp, keypoints_1 )
					positions.append(QRect(cr.x*pyrFactor + kp.pt.x * pyrFactor, cr.y*pyrFactor + kp.pt.y * pyrFactor, 10, 10 ));
			}

		}
		if ( cells.size()>0 )
			value /= cells.size();	
	}
	
	if (metric == 11) //Dense
	{
/*
1	11.1258	2.93258
2	15.4693	1.57822
3	16.3403	3.83005
4	7.92306	0.52953
*/

		cv::DenseFeatureDetector detector;

		std::vector<cv::KeyPoint> keypoints_1;

		positions.clear();

		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			detector.detect( img_1, keypoints_1 );
			value += keypoints_1.size();

			if( keypoints_1.size() )
			{				
				cv::Rect cr = rr.boundingRect();
				positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));

				foreach( cv::KeyPoint kp, keypoints_1 )
					positions.append(QRect(cr.x*pyrFactor + kp.pt.x * pyrFactor, cr.y*pyrFactor + kp.pt.y * pyrFactor, 10, 10 ));
			}

		}
		if ( cells.size()>0 )
			value /= cells.size();	
	}

	if (metric == 12) //Circles
	{
		

		foreach( cv::RotatedRect rr,  cells )
		{			
			cv::Mat img_1 = ce.cellAt( rr.center );
			// smooth it, otherwise a lot of false circles may be detected
			GaussianBlur( img_1, img_1, Size(9, 9), 2, 2 );
			vector<Vec3f> circles;
			HoughCircles(img_1, circles, CV_HOUGH_GRADIENT,
                 2, img_1.rows/4, 200, 100, 5/pyrFactor, 15/pyrFactor );

			value += circles.size();
		}
		if ( cells.size()>0 )
			 value /= cells.size();
	}
	
	
	res->_value = value;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}

//////////////////////////

QString ExifInfo ="Retrieves EXIF value and applies a calculation defined by script.\nThe exif-key in the calculation formula replaces #1-placeholder";
REGISTER_FEATURE( Exif, FeatureExif, ExifInfo );
FeatureExif::FeatureExif()
{
	ParamInfo pi( "exifKey", "EXIF key for retrieving the value from.", "string" );
	_paramInfos.append(pi);

	pi._name= "default";
	pi._info= "Default value, if key not found";
	pi._constrains = "real";
	_paramInfos.append(pi);

	pi._name= "script";
	pi._info= "Script to be applied on value. use #1 to be replaced by retrieved value";
	pi._constrains = "string";
	_paramInfos.append(pi);

}

FeatureExif::~FeatureExif(){}

QString FeatureExif::version()
{
	return "0.0.2";
}

TPFeatureResults FeatureExif::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;
	QString script;
	if (!params.contains("exifKey"))
	{
		//todo error
		return res;
	}

	if (!params.contains("default"))
	{
		//todo error
		return res;
	}

	if (params.contains("script"))
	{
		script = params["script"].toString();
	}

	QString key = params["exifKey"].toString();
	double val = params["default"].toDouble();

	QElapsedTimer timer;
	timer.start();

	QString tval = img.text(key);
	if (!tval.isNull())
		val=tval.toDouble();

	if ( !script.isEmpty() )
	{
		script.replace( "#1", QString("%1").arg(val) );
		val = eval ( script ).toDouble();
	}

	res->_value = val;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;

}


//////////////////////////
QString CellVarianceInfo ="Variance of cell metrics: 0 -number of cells. 1-intensity mean. 2-area";
REGISTER_FEATURE( CellVariance, FeatureCellVariance, CellVarianceInfo );

FeatureCellVariance::FeatureCellVariance()
{
	ParamInfo pi( "1minCell" );
	_paramInfos.append(pi);

	pi._name= "2maxCell";
	_paramInfos.append(pi);

	pi._name= "3param";
	pi._constrains = "[1,2]";
	_paramInfos.append(pi);
}

FeatureCellVariance::~FeatureCellVariance(){}


QString FeatureCellVariance::version()
{
	return "0.0.1";
}


TPFeatureResults FeatureCellVariance::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;
	double val=0;
	if (!params.contains("1minCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("2maxCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("3param"))
	{
		//todo error
		return res;
	}
	
	int pyrFactor = 4; //TOODO optimize,get scaled image from cache

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );

	QElapsedTimer timer;
	timer.start();

	cv::Mat m(_T::toMat(small));
	cv::Mat mo(_T::toMat(img));

	CellExtractor ce;
	int min = params["1minCell"].toInt();
	int max = params["2maxCell"].toInt();
	int param = params["3param"].toInt();
	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	CeMx.lock();
	TRotatedRectList cells = ce.findCellRects( m );
	CeMx.unlock();

	/*QStringList winNames;
	for (int i=0; i<1000; i++)
	{
		winNames.append( QString("Cell Idx:%1").arg(i) );
	}*/

	int cidx=0;
	QList<double> means;
	foreach( cv::RotatedRect rr,  cells )
	{
		cv::Rect cr = rr.boundingRect();
		cr.x*=pyrFactor;
		cr.y*=pyrFactor;
		cr.width*=pyrFactor;
		cr.height*=pyrFactor;
		positions.append(QRect(cr.x, cr.y, cr.width, cr.height));
		cv::Point cc = rr.center; 
		cc.x*=pyrFactor;
		cc.y*=pyrFactor;


		cv::Mat oc = mo(cr);

		float cellMean = cv::mean( oc )[0];

		//normalize( cell, norm, 0,5,CV_MINMAX);
		cv::threshold( oc ,oc,param, 255,  CV_THRESH_TRUNC);
		float thCellMean = cv::mean( oc )[0];
#if 0
		cv::Mat mask = cv::Mat::zeros(oc.size(), CV_8U); // all 0
		cv::Rect roi1(oc.size().width/2 - oc.size().width/4, oc.size().height/2 - oc.size().height/4, oc.size().width/2, oc.size().height/2 );
		mask(roi1) = 1;
		roi1 = cv::Rect(oc.size().width/2 - oc.size().width/8, oc.size().height/2 - oc.size().height/8, oc.size().width/4, oc.size().height/4 );
		mask(roi1) = 0;

		float cellMean = cv::mean( oc, mask )[0];

		cv::Rect ccenter(oc.size().width/2 - oc.size().width/8, oc.size().height/2 - oc.size().height/8, oc.size().width/4, oc.size().height/4 );
		cv::Mat occ = oc(ccenter);

		float centerMean = cv::mean( occ )[0];
#endif
		means.append( thCellMean / cellMean );

		//imshow ( winNames.at(cidx++).toStdString().c_str(), oc );
	}
#if 0
	if ( param == 0 ) //number of cells (not variance!)
	{
		val = cells.size();
	}
	
	//Intensity mean ratios

	if ( param == 1 ) //Center (1/4) to entire
	{	
		QList<double> means;
		cv::Scalar entMean = mean(m);
		cv::Scalar centersMean;
		foreach( cv::RotatedRect rr,  cells )
		{
			int x = rr.center.x-rr.size.width/8;
			int y = rr.center.y-rr.size.height/8;
			int w = rr.size.width/4;
			int h = rr.size.height/4;
			cv::Rect roirect(
					cv::Point(x,y ), 
					cv::Size( w,h ) 
					);

			roirect.x*=


			if(x>0 && y>0 && x+w < mo.cols && y+h < mo.rows)
			{
				cv::Mat roi = mo(roirect);				
				means << cv::mean(roi)[0];
			}
		}
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(means);
		val = meanAndStdDev.second;
	}
	if ( param == 2 ) //std deviation of areas
	{	
		QList<double> areas;
		foreach( cv::RotatedRect rr,  cells )
		{
			int x = rr.center.x-rr.size.width/4;
			int y = rr.center.y-rr.size.height/4;
			int w = rr.size.width/2;
			int h = rr.size.height/2;
			areas.append( w*h );
			cv::Rect roirect(
					cv::Point(x,y ), 
					cv::Size( w,h ) 
					);


			if(x>0 && y>0 && x+w < m.cols && y+h < m.rows)
			{
				cv::Mat roi = m(roirect);
				areas << cv::mean(roi)[0];
			}
		}
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(areas);
		val = meanAndStdDev.second;
	}

	else
	{
		PD_ERROR("Undefined parameter value %d", param);
	}
#endif
	QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(means);
	val = meanAndStdDev.first;
	res->_value = val;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


//////////////////////////
QString CellTextureInfo ="Haralick texture featuers: feature:1-13, d -discance (1-10)";
REGISTER_FEATURE( CellTexture, FeatureCellTexture, CellTextureInfo );

FeatureCellTexture::FeatureCellTexture()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

	pi._name= "feature";
	pi._constrains = "[1-13]";
	_paramInfos.append(pi);

	pi._name= "d";
	pi._constrains = "[1-10]";
	_paramInfos.append(pi);
}

FeatureCellTexture::~FeatureCellTexture(){}


QString FeatureCellTexture::version()
{
	return "0.0.1";
}

/**
 * sum over all glcm.at<float> elements where (i,j), where i+j == n
 */

double condAddSum(int n, cv::Mat & m)
{
  double s = 0.0;
  int w = m.rows;
  int h = m.cols;
  int size = w<h?w:h;

  for (int j=0; j<size; ++j)
  {
    if (2*j == n)
      s += m.at<unsigned char>(j,j);
    for (int i=j+1; i<size; ++i)
      if (i + j == n)
        s += 2 * m.at<unsigned char>(i,j);
  }
  return s;
}




void clearGlcm( cv::Mat & m )
{
	//assert(m.rows() == m.cols());
	int n=m.rows;
	for(int i=0; i<n; i++)
		for(int j=0; j<n; j++)
		{
			m.at<float>(i,j)=0;
		}
}

bool printGlcm( cv::Mat & m, QString fn )
{
	//assert(m.rows() == m.cols());
	int n=m.rows;

	QFile f(fn);
	bool ok = f.open(QIODevice::WriteOnly);
	QTextStream t(&f);
	for(int i=0; i<n; i++)
	{
		t <<"\n";
		for(int j=0; j<n; j++)
		{
			 t << m.at<float>(i,j)<<"\t";

		}
	}
	return ok;
}


void calcGlcm( cv::Mat m, int dist, cv::Mat &glcm )
{
	int v=0;
	int dv90=0;
	int dv45=0;
	int dv135=0;
	int dv=0;
	int x=0;
	int y=0;

	
	cv::Mat glcm0 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm90 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm45 = cv::Mat(glcm.size(), CV_32FC1);
	cv::Mat glcm135 = cv::Mat(glcm.size(), CV_32FC1);

	clearGlcm(glcm);
	clearGlcm(glcm90);
	clearGlcm(glcm45);
	clearGlcm(glcm135);

	try{
		for (y=dist; y<m.cols-dist; ++y)
		{
			for (x=dist; x<m.rows-dist; ++x)
			{
				v = m.at<unsigned char>(x,y);
				dv = m.at<unsigned char>(x+dist,y); //distant value
				dv90 = m.at<unsigned char>(x,y+dist); //distant value
				dv45 = m.at<unsigned char>(x+dist,y+dist); //distant value
				dv135 = m.at<unsigned char>(x-dist,y-dist); //distant value

				glcm.at<float>(v,dv) = glcm.at<float>(v,dv) +1;
				glcm.at<float>(dv,v) = glcm.at<float>(v,dv);
				glcm90.at<float>(v,dv90) = glcm90.at<float>(v,dv90) +1;
				glcm90.at<float>(dv90,v) = glcm90.at<float>(v,dv90);
				glcm90.at<float>(v,dv45) = glcm45.at<float>(v,dv45) +1;
				glcm90.at<float>(dv45,v) = glcm45.at<float>(v,dv45);
				glcm90.at<float>(v,dv135) = glcm135.at<float>(v,dv135) +1;
				glcm90.at<float>(dv135,v) = glcm135.at<float>(v,dv135);
			}
		}
		//printGlcm( glcm, "c:/temp/glcm1.txt" );
		//printGlcm( glcm90, "c:/temp/glcm901.txt" );
		cv::add( glcm,glcm90,glcm );
		//printGlcm( glcm, "c:/temp/glcm.txt" );

		//printGlcm( glcm45, "c:/temp/glcm451.txt" );
		//printGlcm( glcm135, "c:/temp/glcm1351.txt" );
		
		cv::add( glcm,glcm45,glcm );
		cv::add( glcm,glcm135,glcm );
		//printGlcm( glcm, "c:/temp/glcm2.txt" );
		//printGlcm( glcm90, "c:/temp/glcm902.txt" );
		//printGlcm( glcm45, "c:/temp/glcm452.txt" );
		//printGlcm( glcm135, "c:/temp/glcm1352.txt" );
		glcm = glcm/4;

	}
	catch(...)
	{
	
	}
	
	//printGlcm( glcm90, "c:/temp/glcm90.txt" );
	//printGlcm( glcm45, "c:/temp/glcm45.txt" );
	//printGlcm( glcm135, "c:/temp/glcm135.txt" );
	//printGlcm( glcm, n, "c:/temp/glcm.txt" );
	int i;
	i++;
}

int cntNz( cv::Mat & m  )
{
	int n= m.rows;
	int nz = 0;
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<n; j++)
		{
			nz +=m.at<float>(i,j);
		}
	}
	return nz;
}



double condAddSum(cv::Mat & m, int n, int nmax)
{
  double s = 0.0;
  for (int j=0; j<nmax; ++j)
  {
    if (2*j == n)
      s += m.at<float>(j,j);
    for (int i=j+1; i<nmax; ++i)
      if (i + j == n)
        s += 2 * m.at<float>(i,j);
  }
  return s;
}

    /**
     * sum over all matrix elements where (i,j), where i-j == n
     */
double condSubSum(cv::Mat & m, int n, int nmax)
{
  double s = (n == 0) ? m.at<float>(0,0) : 0.0;
  for (int j=0; j<nmax; ++j)
    for (int i=j+1; i<nmax; ++i)
      if (qAbs(i - j) == n)
        s += 2 * m.at<float>(i,j);
  return s;
}


void normGlcm( cv::Mat & m, float num, int n=8  )
{
	for(int i=0; i<n; i++)
		for(int j=0; j<n; j++)
			m.at<float>(i,j) = m.at<float>(i,j) / num;
}

int testglcm()
{
	cv::Mat m(6,6,CV_8UC1);
	m.at<unsigned char>(0,0)=0;m.at<unsigned char>(1,0)=0;m.at<unsigned char>(2,0)=0;m.at<unsigned char>(3,0)=0;m.at<unsigned char>(4,0)=0;m.at<unsigned char>(5,0)=0;
	m.at<unsigned char>(0,1)=0;m.at<unsigned char>(1,1)=1;m.at<unsigned char>(2,1)=4;m.at<unsigned char>(3,1)=3;m.at<unsigned char>(4,1)=4;m.at<unsigned char>(5,1)=0;
	m.at<unsigned char>(0,2)=0;m.at<unsigned char>(1,2)=2;m.at<unsigned char>(2,2)=2;m.at<unsigned char>(3,2)=3;m.at<unsigned char>(4,2)=4;m.at<unsigned char>(5,2)=0;
	m.at<unsigned char>(0,3)=0;m.at<unsigned char>(1,3)=3;m.at<unsigned char>(2,3)=1;m.at<unsigned char>(3,3)=2;m.at<unsigned char>(4,3)=3;m.at<unsigned char>(5,3)=0;
	m.at<unsigned char>(0,4)=0;m.at<unsigned char>(1,4)=4;m.at<unsigned char>(2,4)=2;m.at<unsigned char>(3,4)=4;m.at<unsigned char>(4,4)=4;m.at<unsigned char>(5,4)=0;
	m.at<unsigned char>(0,5)=0;m.at<unsigned char>(1,5)=0;m.at<unsigned char>(2,5)=0;m.at<unsigned char>(3,5)=0;m.at<unsigned char>(4,5)=0;m.at<unsigned char>(5,5)=0;
	const int MaxGl = 5;
	cv::Mat glcm(MaxGl,MaxGl,CV_32FC1);

	clearGlcm( glcm );

	calcGlcm( m, 1,glcm );
	cntNz( glcm );
 return 0;
}

int dummy = testglcm();

TPFeatureResults FeatureCellTexture::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;
	double val=0.0;
	if (!params.contains("minCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("maxCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("feature"))
	{
		//todo error
		return res;
	}

	if (!params.contains("d"))
	{
		//todo error
		return res;
	}
	
	int pyrFactor = 4; //TOODO optimize,get scaled image from cache

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );

	QElapsedTimer timer;
	timer.start();

	cv::Mat m(_T::toMat(small));
	cv::Mat mo(_T::toMat(img));

	CellExtractor ce;
	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();
	int param = params["feature"].toInt();
	int d = params["d"].toInt();
	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	CeMx.lock();
	TRotatedRectList cells = ce.findCellRects( m );
	CeMx.unlock();

	const int MaxGl = 8;
	cv::Mat glcm(MaxGl,MaxGl,CV_32FC1);


	clearGlcm( glcm );

	QList<cv::Rect> cellRects;
	foreach( cv::RotatedRect rr,  cells )
	{
		cv::Rect cr = rr.boundingRect();
		cr.x*=pyrFactor;
		cr.y*=pyrFactor;
		cr.width*=pyrFactor;
		cr.height*=pyrFactor;

		cellRects.append(cr);
		positions.append(QRect(cr.x, cr.y, cr.width, cr.height ));
	}


	QList<double> lasm; // Angular Second Moment (Energy)
	QList<double> lidm; // Inverse Difference Moment (Homogeneity)
	QList<double> lent; // Entropy
	QList<double> lvar; // Variance
	QList<double> lcon;
	QList<double> lcor;
	QList<double> lpro;
	QList<double> lsha;
	QList<double> lsav;
	QList<double> lsva;
	QList<double> lsen;
	QList<double> ldav;
	QList<double> lcov;

	foreach( cv::Rect rr,  cellRects )
	{
		
		int x = rr.x+rr.width/4;
		int y = rr.y+rr.height/4;
		int w = rr.width/2;
		int h = rr.height/2;
		cv::Rect roirect(
				cv::Point(x,y ), 
				cv::Size( w,h ) 
				);

		


		if(x>0 && y>0 && x+w < mo.cols && y+h < mo.rows)
		{
			cv::Mat roi = mo(roirect);
			cv::normalize(roi, roi,0,MaxGl-1, CV_MINMAX);


			clearGlcm( glcm );
			calcGlcm( roi, d, glcm );

			int num = cntNz(glcm);
			normGlcm(glcm,num);
			

			double avg = cv::mean(glcm)[0];
			double vasm=0.0; //F1 Angular Second Moment
			double vcon=0.0; //F2 Contrast
			double vidm=0.0; //
			double vent=0.0;
			double vvar=0.0;
			double vcor=0.0; //F3 Correlation
			double vpro=0.0;
			double vsha=0.0;
			double vsav=0.0;
			double vsva=0.0;
			double vsen=0.0;
			double vdav=0.0;
			double vcov=0.0;

		  

			// calculate the mean (average)
      double average = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        average += j*glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          average += 2*j*glcm.at<float>(i,j);
      }

      // calculate the variance
      double variance = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        double var_i = glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          var_i += 2 * glcm.at<float>(i,j);
        variance += var_i * ((j - average)*(j - average));
      }


	  if(param == 1 )
		{
		  double res = 0.0;
		  for (int j=0; j<MaxGl; ++j)
		  {
			res += (glcm.at<float>(j,j))*(glcm.at<float>(j,j));
			for (int i=j+1; i<MaxGl; ++i)
			  res += 2 * ((glcm.at<float>(j,i))*(glcm.at<float>(j,i)));
		  }
		  val = res;
		}
	 /**
     * Inverse Difference Moment (Homogeneity}
     */
    else if(param == 2 )
    {
      double res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * (glcm.at<float>(j,i) / (1 + ((i - j)*(i - j))));
      }
      val = res;
    }

	/**
     * Entropy
     */
    else if (param == 3 )
    {
      double res = 0.0;
      double shift = 0.0001;
      for (int j=0; j<MaxGl; ++j)
      {
        res += glcm.at<float>(j,j) * qLn(glcm.at<float>(j,j) + shift);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * glcm.at<float>(j,i) * log(glcm.at<float>(j,i) + shift);
      }
      val =  (-1) * res;
    }

	/**
     * Variance
     */
     else if (param == 4 )
    {
      double res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += glcm.at<float>(j,j) * ((j - average)*(j - average));
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * glcm.at<float>(i,j) * ((j - average)*(j - average));
      }
      val = res;
    }

	/**
     * Contrast (Interia)
     */
    else if (param == 5 )
    {
      double res = 0.0;
      for (int j=0; j<MaxGl; ++j)
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * glcm.at<float>(i,j) * ((i - j)*(i - j));
      val = res;
    }

    /**
     * Correlation
     */
    else if (param == 6 )
    {
      double res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += ((j - average)*(j - average)) * glcm.at<float>(j,j) / (variance*variance);
        for (int i=j+1; i<MaxGl; ++i)
          res += (2 * (i - average) * (j - average) *  glcm.at<float>(i,j) /
                  (variance*variance));
      }
      val= res;
    }
	/**
     * Prominence
     */
    else if (param == 7 )
    {
      double res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += pow((2 * j - 2 * average),4) * glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * qPow((i + j - 2 * average),4) * glcm.at<float>(i,j);
      }
      val=  res;
    }

    /**
     * Shade
     */
    else if (param == 8 )
    {
      double res = 0.0;
      for (int j=0; j<MaxGl; ++j)
      {
        res += pow((2 * j - 2 * average),3) * glcm.at<float>(j,j);
        for (int i=j+1; i<MaxGl; ++i)
          res += 2 * qPow((i + j - 2 * average),3) * glcm.at<float>(i,j);
      }
      val=  res;
    }

    /**
     * Sum Average
     */
    else if (param == 9 )
    {
      double res = 0.0;
      for (int n=2; n <= 2*MaxGl; ++n)
        res += n * condAddSum(glcm,n,MaxGl);
      val=  res;
    }

    /**
     * Sum Variance
     */
    else if (param == 10 )
    {
      double res = 0.0;
      for (int n=2; n <= 2*MaxGl; ++n)
        res += n * condAddSum(glcm,n,MaxGl);

	  double sav = res;
	  res = 0.0;
      
      for (int n=2; n <= 2*MaxGl; ++n)
        res += ((n - sav)*(n - sav)) * condAddSum(glcm,n,MaxGl);
      val=  res;
    }

    /**
     * Sum Entropy
     */
    else if (param == 11 )
    {
      double res = 0.0;
      for (int n=2; n <= 2*MaxGl; ++n)
        res += n * condAddSum(glcm,n,MaxGl);

	  double sav = res;
	  res = 0.0;
      double shift = 0.0001;
      for (int n=2; n <= 2*MaxGl; ++n)
      {
        double cas = condAddSum(glcm,n,MaxGl) + shift;
        res += cas * log(cas);
      }
      val=  (-1) * res;
    }

    /**
     * Difference Average
     */
    else if (param == 12 )
    {
      double res = 0.0;
      for (int n=0; n < MaxGl; ++n)
        res += n * condSubSum(glcm,n,MaxGl);
      val=  res;
    }

    /**
     * Coefficient of Variation
     */
    else if (param == 13 )
    {
      val=  variance / average;
    }
	lasm.append(val);

#if 0
		if( param == 3 ||param == 4 )
		{
		  for (int i = 0; i < MaxGl; ++i)
		  {
			for (int j = 0; j < MaxGl; ++j)
			{
				float v_ij = glcm.at<float>(i,j);
				vasm += v_ij * v_ij;
				vent += v_ij * qLn(v_ij + 0.0001);//res += glcm.at<float>(j,j) * log(glcm.at<float>(j,j) + shift);
				vvar += v_ij * ( (j - avg)* (j - avg) );//glcm.at<float>(j,j) * sqr((j - average));
			}
		  }
		}

		if( param == 2 )
		{
		  float sum = 0, entsum = 0;
		  for (int n = 0; n < MaxGl; ++n)
		  {
			  for (int i = 0; i < MaxGl; ++i)
			  {
				for (int j = 0; j < MaxGl; ++j)
				{
					float v_ij = glcm.at<float>(i,j);
					if ((i - j) == n || (j - i) == n)
					{
						sum += v_ij;
					}
				}

			  }
			  entsum += n * n * sum;
		  }
		  vcon = entsum;
		}
#endif
		  

			/*
			int i, j, n;
  float sum = 0, bigsum = 0;

  for (n = 0; n < Ng; ++n)
  {
    for (i = 0; i < Ng; ++i)
      for (j = 0; j < Ng; ++j)
	if ((i - j) == n || (j - i) == n)
	  sum += P[i][j];
    bigsum += n * n * sum;

    sum = 0;
  }
  return bigsum;
*/



			/*
			for (int j=1; 0 && j<MaxGl; ++j)
			{
				float val = glcm.at<float>(j,j);
				vasm += val * val;
				vidm += val;
				vent += val * qLn(val) + 0.0001;//res += glcm.at<float>(j,j) * log(glcm.at<float>(j,j) + shift);
				vvar += val * ( (j - avg)* (j - avg) );//glcm.at<float>(j,j) * sqr((j - average));
				//vcon; //
				//vcor += qSqrt( j - avg ) * roi.at<unsigned char>(j,j) / qSqrt(avg);// sqr((j - average)) * glcm.at<float>(j,j) / sqr(variance);
				for (int i=j+1; i<MaxGl; ++i)
				{
					float vald = glcm.at<float>(j,i);
					float valad = glcm.at<float>(i,j);
					vasm += 2 * (vald*vald);                //2 * sqr(glcm.at<float>(j,i));
					vidm += 2 * vald / (1+qSqrt(i-j));      //2 * (glcm.at<float>(j,i) / (1 + sqr((i - j))));
					vent += 2 * vald * (qLn(vald)+0.0001);     //2 * glcm.at<float>(j,i) * log(glcm.at<float>(j,i) + shift);
					vvar += 2 * valad * qSqrt(j-avg); // 2 * glcm.at<float>(i,j) * sqr((j - average));                    
					vcon += 2 * valad * qSqrt(i-j); //2 * glcm.at<float>(i,j) * sqr((i - j));
					//vcor += 2 * qSqrt(roi.at<unsigned char>(j,j));
					vpro += 2 * qPow((i+j-2*avg),4) * vald;// 2 * pow((i + j - 2 * average),4) * glcm.at<float>(i,j);
					vsha += 2 * qPow((i+j-2*avg),3) * vald;//2 * pow((i + j - 2 * average),3) * glcm.at<float>(i,j);


				}
			}
			*/
		/*
			lasm.append(vasm);
			lidm.append(vidm);
			lent.append(vent);
			lvar.append(vvar);
			lcon.append(vcon);
			lcor.append(vcor);
			lpro.append(vpro);
			lsha.append(vsha);
*/

		}

	}
/*
	if ( param == 1 ) //ASM(angular second moment)
	{			
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lasm);
		val = meanAndStdDev.first;
	}

	else if ( param == 2 ) //Inverse Difference Moment (Homogeneity)
	{			
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lidm);
		val = meanAndStdDev.first;
	}

	else if ( param == 3 ) //Entropy
	{			
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lent);
		val = meanAndStdDev.first;
	}

	else if ( param == 4 ) //Variance
	{			
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lvar);
		val = meanAndStdDev.first;
	}

	else if ( param == 5 ) //Contrast (Interia)
	{			
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lcon);
		val = meanAndStdDev.first;
	}
	// 6 Correlation
	else if ( param == 7 ) //Prominence
	{			
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lpro);
		val = meanAndStdDev.first;
	}
	else if ( param == 8 ) //Shade
	{			
		QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lsha);
		val = meanAndStdDev.first;
	}

	else
	{
		PD_ERROR("Undefined parameter value %d", param);
	}
*/
	
	QPair<double,double> meanAndStdDev = PDImageProcessing::getMeanAndDev(lasm);
	val = meanAndStdDev.first;
	res->_value = val;

	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}


//Cell Form

//////////////////////////
QString CellShapeInfo ="Shape of cell metrics: 1-Connected Regions";
REGISTER_FEATURE( CellShape, FeatureCellShape, CellShapeInfo );

FeatureCellShape::FeatureCellShape()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

	pi._name= "param";
	pi._constrains = "[1]";
	_paramInfos.append(pi);
}

FeatureCellShape::~FeatureCellShape(){}


QString FeatureCellShape::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureCellShape::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;
	double val=0.0;
	if (!params.contains("minCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("maxCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("param"))
	{
		//todo error
		return res;
	}
	
	int pyrFactor = 4;

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );

	QElapsedTimer timer;
	timer.start();

	cv::Mat m(_T::toMat(small));
	cv::Mat mo(_T::toMat(img));

	CellExtractor ce;
	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();
	int param = params["param"].toInt();
	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	CeMx.lock();
	TRotatedRectList cells = ce.findCellRects( m );
	CeMx.unlock();

	foreach( cv::RotatedRect rr,  cells )
	{
		cv::Rect cr = rr.boundingRect();
		positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));

	}


	float numOfRegions=0;

	float areasRatioNz=0;
	int cellCnt=0;
	foreach( cv::RotatedRect rr,  cells )
	{
		
		int x = (rr.center.x-(rr.size.width/2))*pyrFactor;
		int y = (rr.center.y-(rr.size.height/2))*pyrFactor;
		int w = (rr.size.width)*pyrFactor;
		int h = (rr.size.height)*pyrFactor;
		cv::Rect roirect(
				cv::Point(x,y ), 
				cv::Size( w,h ) 
				);

		int idx =0;		


		
		if(x>0 && y>0 && x+w < m.cols && y+h < m.rows)
		{
			Mat cell = ce.cellAt/*normalizedCellAt*/( rr.center,/* cv::Size( 100,100 ),*/ &idx );
			if ( idx > 0 )
			{
				Mat norm = Mat::zeros( cell.rows, cell.cols, CV_8UC1 );
				Mat mask = Mat::zeros( cell.rows, cell.cols, CV_8UC1);
				normalize( cell, norm, 0,5,CV_MINMAX);
				cv::threshold( norm ,mask,param, 255,  CV_THRESH_BINARY);
				areasRatioNz += (float)cv::countNonZero(mask) / (mask.rows * mask.cols);
				cellCnt++;
				
			}

			//cv::Mat roi = mo(roirect);
			
			
		
			/*
			cv::threshold( norm ,mask,4, 255,  CV_THRESH_BINARY);
			imshow("cell4", mask);
			cv::threshold( norm ,mask,3, 255,  CV_THRESH_BINARY);
			imshow("cell3", mask);
			cv::threshold( norm ,mask,2, 255,  CV_THRESH_BINARY);
			imshow("cell2", mask);
			*/
		}
	}

	res->_value = areasRatioNz / cellCnt;
	res->_positions = positions;


	
	return res;
}


//Cell Form
//////////////////////////
QString FindTemplInfo ="Finds template regions";
REGISTER_FEATURE( FindTempl, FeatureFindTempl, FindTemplInfo );

FeatureFindTempl::FeatureFindTempl()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

	pi._name= "meth";
	pi._constrains = "[0-5, 10]";
	_paramInfos.append(pi);

	pi._name= "tmpl";
	_paramInfos.append(pi);

}

FeatureFindTempl::~FeatureFindTempl(){}


QString FeatureFindTempl::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureFindTempl::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	QImage testImg;
	double val=0.0;
	if (!params.contains("minCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("maxCell"))
	{
		//todo error
		return res;
	}

	if (!params.contains("meth"))
	{
		//todo error
		return res;
	}
	
	if (!params.contains("tmpl"))
	{
		//todo error
		return res;
	}

	int pyrFactor = 4;

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );

	
	QElapsedTimer timer;
	timer.start();

	cv::Mat m(_T::toMat(small));
	cv::Mat mo(_T::toMat(img));

	CellExtractor ce;
	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();
	int meth = params["meth"].toInt();
	QString tmplFn = params["tmpl"].toString();

	QImage tmpl(QString( "c:/aeskuimg/tmpl/%1.png" ).arg( tmplFn ) );
	cv::Mat mt = _T::toMat(tmpl);

	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	CeMx.lock();
	TRotatedRectList cells = ce.findCellRects( m );
	CeMx.unlock();

	int cidx=0;
	foreach( cv::RotatedRect rr,  cells )
	{
		cv::Rect cr = rr.boundingRect();
		cr.x *=pyrFactor;
		cr.y *=pyrFactor;
		cr.width *=pyrFactor;
		cr.height *=pyrFactor;
		cv::Mat cell = mo(cr);
		QString imageName = QString( "Cell %1" ).arg(cidx++);


		res->_value = PDImageProcessing::findItems( cell, mt, meth ).size();


		positions.append(QRect(cr.x, cr.y, cr.width, cr.height ));

	}
	
   return res;
}

