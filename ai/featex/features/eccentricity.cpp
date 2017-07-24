#include "eccentricity.h"
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
//#include <opencv2/ml/ml.hpp>


using namespace cv;


QString EccentricityInfo ="Ratio of the length of major axis to the length of minor axis.\n"
 "(minimum bounding rectangle method)\n'metric'; \n0 - mean, 1 - variance";
REGISTER_FEATURE( Eccentricity, FeatureEccentricity, EccentricityInfo );

FeatureEccentricity::FeatureEccentricity()
{
	ParamInfo pi( "minCell" );
	_paramInfos.append(pi);

	pi._name= "maxCell";
	_paramInfos.append(pi);

    pi._name= "metric"; //0 - mean, 1 - variance
	_paramInfos.append(pi);

}

FeatureEccentricity::~FeatureEccentricity(){}

QString FeatureEccentricity::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureEccentricity::extract( QImage & img, TFeatParams params )
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

	int pyrFactor = 8; 

	QImage small = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor, img.height()/pyrFactor) );
	cv::Mat m(_T::toMat(small));

	CellExtractor ce;
	int min = params["minCell"].toInt();
	int max = params["maxCell"].toInt();

	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);

	QString hk = small.text( "hashkey" );

	TRotatedRectList cells = ce.findCellRects( m );
	
	QList<double> values;

	foreach( cv::RotatedRect rr,  cells )
	{
		double maj = rr.size.width > rr.size.height?rr.size.width : rr.size.height;
		double min = rr.size.width > rr.size.height?rr.size.height : rr.size.width;
		values.append( maj/min );
		cv::Rect cr = rr.boundingRect();
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