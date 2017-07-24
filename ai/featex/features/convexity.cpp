#include "Convexity.h"
#include <QImage>
#include <pdimageprocessing.h>
#include <cells.h>
#include <vector>
#include <QtCore/qmath.h>
#include <QElapsedTimer>

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace cv;


QString ConvexityInfo ="Ratio of perimeters of the convex hull over the shape contour";
REGISTER_FEATURE( Convexity, FeatureConvexity, ConvexityInfo );

FeatureConvexity::FeatureConvexity()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

    pi._name= "metric"; //0 - mean, 1 - variance
	_paramInfos.append(pi);

}

FeatureConvexity::~FeatureConvexity(){}

QString FeatureConvexity::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureConvexity::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res(new FeatureResults(-1));
	QList<QRect> positions;

	int minNum=30;
	int metric = 0;


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

	if (params.contains("metric"))
	{
		metric = params["metric"].toInt();
	}


	QElapsedTimer timer;
	timer.start();

	int pyrFactor = 4; 

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
	cv::Mat m(_T::toMat(small));

	CellExtractor ce;
	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();

	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	typedef std::vector<cv::Point> TContour;
	

	QString hk = small.text( "hashkey" );

	TRotatedRectList cells = ce.findCellRects( m );
	std::vector< TContour > conts = ce.cellContours();
	
	QList<double> values;

	foreach( TContour c,  conts )
	{
		double Ps = cv::arcLength(c,true);
		TContour ch;
		cv::convexHull(c,ch);
		double Ph = cv::arcLength(ch,true);
		values.append( Ph/Ps );
		cv::Rect cr = boundingRect(c);
		positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
	}

	QPair<double, double> result = PDImageProcessing::getMeanAndDev( values );

	if ( metric == 0 ) //mean
	{
		res->_value = result.first;
	}
	else if ( metric == 1 ) //variance
	{
		res->_value = result.second;
	}
	else
	{
		PD_ERROR("Undefine metric %d", metric);
	}


	res->_positions = positions;
	res->_extractionTimeMs =timer.elapsed();
	return res;
}