#include "featex.h"
#include <QStringList>
#include <QDir>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QSqlRecord>
#include <QPainter>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QtCore>
#include <QMutexLocker>
#include <QFileInfo>
#include <pdlog.h>
#include <QSqlError>
#include <QImage>
#include <QList>
#include <QRect>

#include <pdimageprocessing.h>
#include <cells.h>

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
//#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/nonfree/nonfree.hpp"

using namespace cv;

QString DBConnectionName = "FeatEx_01";

bool FeatEx::createDatabase( QString path, bool useExisting )
{
	
	if ( QFileInfo(path).exists() && !useExisting )
	{		
		PD_ERROR("Database already exists in: %s", QS2CS(path));
		return false;
	}

	QMutexLocker dbml(&_dbMx);
    QStringList drvs =  _database.drivers();

	unsigned long dbud = (unsigned long)this;

	_database = QSqlDatabase::addDatabase("QSQLITE",QString("FB_FEATEX_%1").arg(dbud));
	_database.setDatabaseName( path );
	 if (!_database.isValid())
	 {
		 PD_ERROR("Database invalid in: %s, DBMSG:%s, DRVMSG:%s", QS2CS(path), QS2CS(_database.lastError().databaseText()), QS2CS(_database.lastError().driverText()));
		 //return false;
	 }

     if (!_database.open()) {
		 PD_ERROR("Database IO error: %s", QS2CS(path));
		 PD_ERROR(" DBMSG:%s, DRVMSG:%s",  QS2CS(_database.lastError().databaseText()), QS2CS(_database.lastError().driverText()));
         return false;
     }

     QSqlQuery query(_database);
	 bool ok = query.exec("CREATE TABLE IF NOT EXISTS FeatureSampleValue (Id INTEGER PRIMARY KEY AUTOINCREMENT, FeatureRef varchar(50), SampleRef varchar(255), Value REAL, Performance INTEGER)");
	 if (!ok)
	 {
		 PD_ERROR(" Database command error while trying to create. DBMSG:%s, DRVMSG:%s",  QS2CS(_database.lastError().databaseText()), QS2CS(_database.lastError().driverText()));
		 return false;
	 }

	 ok = query.exec("SELECT COUNT (Id) FROM FeatureSampleValue");
	 if (!ok)
	 {
		 PD_ERROR(" Database command error while trying to create. DBMSG:%s, DRVMSG:%s",  QS2CS(_database.lastError().databaseText()), QS2CS(_database.lastError().driverText()));
		 return false;
	 }

	 query.next();
	 int rows = query.value(0).toInt();
	 int maxRows = 100000;
	 if (rows > maxRows)
	 {
		ok = query.exec( QString("DELETE FROM FeatureSampleValue WHERE Id IN ( SELECT Id FROM FeatureSampleValue ORDER BY Id  LIMIT %1 )").arg(rows/2));
		if (!ok)
		{
		 PD_ERROR(" Database command error while trying to create. DBMSG:%s, DRVMSG:%s",  QS2CS(_database.lastError().databaseText()), QS2CS(_database.lastError().driverText()));
		 return false;
		}
	 }

	 return true;
 }

void FeatEx::clearAllStatistics( )
{
	QMutexLocker dbml(&_dbMx);
	QSqlQuery q(_database);
	QString sql = " DELETE FROM FeatureSampleValue "; 
    if( !q.exec( sql ) )
    {
		PD_ERROR("Database command error: %s", QS2CS(sql));
    }
}


FeatEx::FeatEx(QObject *parent) :
    QObject(parent),
	_dbMx( QMutex::Recursive),
	_futureCnt(0),
	_stop(0)
{
	//PD_LOG("Feature extructor created" )
}

FeatEx::~FeatEx()
{	
	//PD_LOG("Feature extructor shut down" )
	_runningFuturesMx.lock();
	foreach( QFuture<double> f, _runningFutures )
	{
		f.cancel();
	}
	_runningFuturesMx.unlock();
	//_deleteMx.lock(); // make sure no other thread is using my methods
	//_deleteMx.unlock();
}

bool FeatEx::extractSampleFeatures( QString sampleRef, QStringList features, bool concurrent )
{
	//PD_LOG("%s; %s", QS2CS(sampleRef), QS2CS(features.join(",")) )
	if ( !QFileInfo(sampleRef).exists() )
	{
		PD_ERROR("Can't extract features (%s). No such instance:  %s ", QS2CS(sampleRef), QS2CS(features.join("; ")) );
		return false;
	}
	
	//QMutexLocker dbml(&_dbMx);
	QFutureSynchronizer<double> synchronizer;

	bool checkExists = true;
	foreach( QString f, features )
	{
		if ( _stop ) return false; //user stop

		++_futureCnt;
		QList<QRect> *dummy=0;
		QFuture<double> fres = QtConcurrent::run( this, &FeatEx::getSampleFeatureValue, sampleRef, f, dummy );
		_runningFuturesMx.lock();
		_runningFutures[_futureCnt] = fres ;
		_runningFuturesMx.unlock();

		if ( concurrent ) // in asynchronous mode notify finished by signal
		{
			QFutureWatcher<double> * fw = new QFutureWatcher<double>(this);
			fw->setProperty( "imageReference", sampleRef  );
			fw->setProperty("futureId",_futureCnt);
			connect(fw, SIGNAL(finished()), this, SLOT(processExtractionFinished()), Qt::QueuedConnection);
			fw->setFuture( fres );
			
			if( checkExists && !_featureSets.contains( sampleRef ) )
			{
				_featureSets[sampleRef] = TPFeatureSetResults( new FeatureSetResulfs );
				_featureSets[sampleRef]->_featureSet = features;
			}
			_featureSets[sampleRef]->_inExtraction.append( fw );
		}
		else
		{
			synchronizer.addFuture(fres);
		}
	}

	if( !concurrent )
	{
		synchronizer.waitForFinished();
	}
	return true;
}

void FeatEx::processExtractionFinished()
{
	QObject *s = sender();
	QString imgref = s->property("imageReference").toString();
	TPFeatureSetResults fsr = _featureSets[imgref];

	//PD_LOG( "finished: %s", QS2CS(imgref) )

	int futureId = s->property("futureId").toInt();
	_runningFuturesMx.lock();
	this->_runningFutures.remove(futureId);
	_runningFuturesMx.unlock();

	foreach( QFutureWatcher<double>* fe, _featureSets[imgref]->_inExtraction )
	{
		if (!fe->isFinished())
			return;
	}

	//finished, delete future objects
	foreach( QFutureWatcher<double>* fe, _featureSets[imgref]->_inExtraction )
	{
		fe->deleteLater();
	}

	emit sampleFeaturesExtracted(imgref, _featureSets[imgref]->_featureSet );
	_featureSets.remove(imgref);
}

bool FeatEx::stopCalculation()
{
	//PD_LOG( "Stop" )
	_stop=1;
	foreach( TPFeatureSetResults fs, _featureSets.values() )
	{
		foreach( QFutureWatcher<double>* fe, fs->_inExtraction )
		{
			if (!fe->isFinished())
				fe->pause();
		}
	}

	_runningFuturesMx.lock();
	foreach( QFuture<double> f, _runningFutures )
	{
		f.cancel();
	}
	_runningFutures.clear();
	_runningFuturesMx.unlock();

	_featureSets.clear();
	return true;
}



QList<QRect> FeatEx::getHomMitosisRegions( QImage img, int min, int max )
{
	int pyrFactor = 4; //TOODO optimize,get scaled image from cache
	QList<QRect> positions;

	QImage pdimg = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor,img.height()/pyrFactor) );
	cv::Mat m(_T::toMat(pdimg));

	CellExtractor ce( min/pyrFactor, max/pyrFactor, 7, 9, 10 );
	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	TRotatedRectList cells = ce.findCellRects1( m  );


	foreach( cv::RotatedRect rr,  cells )
	{
		cv::Rect cr = rr.boundingRect();
		positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
	}
	return positions;
}


QList<QRect> FeatEx::getCellRegions( QImage img, int min, int max, int minCells, int sim, int roi )
{
	int pyrFactor = 4; //TOODO optimize,get scaled image from cache
	QList<QRect> positions;

	QImage pdimg = ImageCache::instance().getImage(img, QSize(img.width()/pyrFactor,img.height()/pyrFactor) );
	cv::Mat m(_T::toMat(pdimg));

	CellExtractor ce( min/pyrFactor, max/pyrFactor, 1, 4, 5 );
	ce.setMinCell(min/pyrFactor);
	ce.setMaxCell(max/pyrFactor);
	TRotatedRectList cells = ce.findCellRects( m, minCells, sim, roi );


	foreach( cv::RotatedRect rr,  cells )
	{
		cv::Rect cr = rr.boundingRect();
		positions.append(QRect(cr.x*pyrFactor, cr.y*pyrFactor, cr.width*pyrFactor, cr.height*pyrFactor ));
	}
	return positions;
}


double FeatEx::getSampleFeatureValue( QString sampleRef, QString featureRef, QList<QRect> * positions )
{
	if( _stop)
		return 0;	
	
	TFeatParams params;
	QString featureName;
	QString fver;

	splitFeatureName( featureRef, featureName, fver, params );
	// (Id INTEGER PRIMARY KEY AUTOINCREMENT, FeatureRef varchar(50), SampleRef varchar(255), Value REAL
	double val=0.0;

	QString qs = QString(
		"SELECT FeatureRef, SampleRef, Value FROM FeatureSampleValue WHERE "
		"FeatureRef='%1' AND SampleRef LIKE '%%2%' "
		)
		.arg( featureRef )
		.arg( sampleRef );

	_dbMx.lock();
	if( _stop)
	{
		_dbMx.unlock();
		return 0;
	}
	QSqlQuery q( _database );	
	bool ok = q.exec(qs);
	_dbMx.unlock();
	
	if(  ok && q.next() )
	{
		val = q.value(2).toDouble();
	}
	else
	{
		QImage tmpImg;
		_usedImagesMx.lock();
		if ( !_usedImages.contains(sampleRef) )
		{
			_usedImagesMx.unlock();

			tmpImg = QImage( sampleRef );

			_usedImagesMx.lock();
			_usedImages[sampleRef] = tmpImg;
			_usedImagesQueue.append( sampleRef );

			if ( _usedImagesQueue.size() > 10 ) // TODO as parameter. Max number of images stored temporarily in queue
			{
				_usedImages.remove(_usedImagesQueue.first());
				_usedImagesQueue.takeFirst();
			}
		}

		tmpImg = ( _usedImages[sampleRef] );		
		_usedImagesMx.unlock();		
		
		Feature::evalParams( tmpImg,params );		
		TPFeature tmpf = FeatureManager::instance().getFeature(featureName);
		if (!tmpf.get())
		{
			PD_ERROR("Internal error");
			return 0;
		}		

		TPFeatureResults fval = tmpf->extract(tmpImg, params);
		if (!fval)
		{
			PD_ERROR("Internal error");
			return 0;
		}
		val = fval->_value;		

		QString sval = QString("%1").arg(val);
		if (sval == "nan")
			val = 0;

		if( positions )
			*positions = fval->_positions;

		QString sql = QString("INSERT INTO FeatureSampleValue "
						 "(FeatureRef, SampleRef, Value, Performance) "
						 " VALUES ('%1', '%2', %3, %4)" )
						 .arg(featureRef)
						 .arg(sampleRef)
						 .arg(val)
						 .arg(fval->_extractionTimeMs);

		_dbMx.lock();
		if( _stop)
		{
			_dbMx.unlock();
			return 0;
		}
		QSqlQuery iq(_database);
		bool ok = iq.exec(sql);
		_dbMx.unlock();
		if( !ok )
		{
			qDebug()<<"Database write error";
		}				
	}
	return val;
}


QString FeatEx::autoFeatureName(  QString fn, TFeatParams fp)
{
	QString featureName = fn;
	TPFeature f= FeatureManager::instance().getFeature( featureName );
	if (!f.get())
		return "";
	featureName += QString("_%1").arg(f->version());
	for( TFeatParams::const_iterator it= fp.begin(), end= fp.end(); it!=end; ++it )
	{
		featureName += QString("_%2:%3").arg(it.key()).arg(it.value().toString());
	}

	return featureName;
}
