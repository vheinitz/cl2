#include "scriptfeature.h"
#include "ipscript.h"

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

QString Script ="Feature extraction using a generic script";
REGISTER_FEATURE( Script, FeatureScript, Script );

FeatureScript::FeatureScript()
{
	ParamInfo pi( "name" );
	_paramInfos.append(pi);

	pi._name = "script" ;
	_paramInfos.append(pi);
}

FeatureScript::~FeatureScript(){}

QString FeatureScript::version()
{
	return "0.0.1";
}

TPFeatureResults FeatureScript::extract( QImage & img, TFeatParams params )
{
	TPFeatureResults res( new FeatureResults(-1) );
	QList<QRect> positions;

	QString name, script;
	
	if ( !params.contains("name") )
	{
		return res;
	}

	if ( !params.contains("script") )
	{
		return res;
	}

	QElapsedTimer timer;
	timer.start();

	name = params["name"].toString( );
	script = params["script"].toString( );

	ImageProcessingScriptEngine *se = new ImageProcessingScriptEngine();
	ScriptValue sv = se->exec( img, script );
	
	if( sv._type == EValues && sv._values.size() == 1 )
	{
		res->_value = sv._values.at(0);
	}

	return res;
}
