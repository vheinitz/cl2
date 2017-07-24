#include "feature.h"

#include <QRegExp>
#include <QScriptEngine>
#include <QScriptProgram>
#include <QScriptValue>
#include "pdimageprocessing.h"

struct ReverseSorter { bool operator()( QString s1, QString s2 ){ return s1.length() > s2.length(); } };

QMutex _imgInfoGuard;

bool Feature::evalParams( QImage img, TFeatParams &params )
{
	PDImageInfo iminf;
	PDImageProcessing::getImageInfo(img, iminf);

	for( TFeatParams::iterator it = params.begin(); it != params.end(); ++it   )
	{

		QString script = it.value().toString();
		if (!script.contains("$")) //TODO ???
			continue;

		QStringList vars;
		QRegExp varRx ("\\$[A-Za-z_.0-9]+");
		int lastIdx = varRx.indexIn (script, 0);
		while (lastIdx != -1)
		{
			vars.append (script.mid (lastIdx, varRx.matchedLength ()));
			lastIdx += varRx.matchedLength ();
			lastIdx = varRx.indexIn (script, lastIdx);
		}
		qSort(vars.begin(), vars.end(), ReverseSorter() );

		foreach( QString v, vars ) script.replace(v, QString::number(iminf.value(v.mid(1))) );
		QString sresult="";
		if ( !script.isEmpty() )
		{
			if ( script.at(0)=='"' && script.length()>=2 )
			{
				sresult = script.mid(1,script.length()-2);
			}
			else
			{
				QScriptEngine engine;
				PD_TRACE ("FEATURETEST SCRIPT SRC(PREP): %s", QS2CS (script))
				QScriptValue result = engine.evaluate (QScriptProgram (script));
				sresult = result.toString ();
				if (result.isError ())
				{
					PD_ERROR( "Error in Feature-parameter: %s, Result:", QS2CS(script), QS2CS(sresult) )
				}
			}
		}
		it.value() = sresult.toDouble(); //TODO ???
		PD_TRACE ("FEATURETEST SCRIPT RESULT: %s", QS2CS(sresult))
	}
	return true;
}

double Feature::eval( QImage img, const QString & expression ) const
{
	PDImageInfo iminf;
	PDImageProcessing::getImageInfo(img, iminf);

	QString script = expression;
	QStringList vars;
	QRegExp varRx ("\\$[A-Za-z_.0-9]+");
	int lastIdx = varRx.indexIn (script, 0);
	while (lastIdx != -1)
	{
		vars.append (script.mid (lastIdx, varRx.matchedLength ()));
		lastIdx += varRx.matchedLength ();
		lastIdx = varRx.indexIn (script, lastIdx);
	}
	qSort(vars.begin(), vars.end(), ReverseSorter() );

	foreach( QString v, vars ) script.replace(v, QString::number(iminf.value(v.mid(1))) );
	QString sresult="";
	if ( !script.isEmpty() )
	{
		if ( script.at(0)=='"' && script.length()>=2 )
		{
			sresult = script.mid(1,script.length()-2);
		}
		else
		{
			QScriptEngine engine;
			PD_TRACE ("FEATURETEST SCRIPT SRC(PREP): %s", QS2CS (script))
			QScriptValue result = engine.evaluate (QScriptProgram (script));
			sresult = result.toString ();
			if (result.isError ())
			{
				PD_ERROR( "Error in Feature-parameter: %s, Result:", QS2CS(script), QS2CS(sresult) )
			}
		}
	}
	PD_TRACE ("FEATURETEST SCRIPT RESULT: %s", QS2CS(sresult))
	return sresult.toDouble();
}

QString Feature::eval( const QString & script ) const
{
	if ( !script.isEmpty() )
	{
		QScriptEngine engine;
		PD_TRACE ("FEATURETEST SCRIPT SRC(PREP): %s", QS2CS (script))
		QScriptValue result = engine.evaluate (QScriptProgram (script));			
		if (!result.isError ())
		{
			return result.toString ();
		}
	}
	return QString::null;
}
