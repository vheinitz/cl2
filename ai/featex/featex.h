#ifndef FEATURE_EXTRACTOR_HG__H
#define FEATURE_EXTRACTOR_HG__H

#include <QSqlDatabase>
#include <QImage>
#include <pdimageprocessing.h>
#include <QMutex>
#include <QMap>
#include <QQueue>

#include "features/feature.h"

#include <QFuture>
#include <QFutureWatcher>

#ifdef BUILDING_FEATEX_DLL
# define FEATEX_EXPORT Q_DECL_EXPORT
#else
# define FEATEX_EXPORT Q_DECL_IMPORT
#endif

#ifdef BUILDING_FEATEX_EMBEDDED
# undef FEATEX_EXPORT
# define FEATEX_EXPORT
#endif


/* TODO: describe features as JSON
{ "FID": "Feature-ID", "FV": "Version", "IOP": [ ["operation1","av1"],["operation2","av1","av2","avN"] ], 
"OS":[["os1","av1","av2","av3"],["os1","av1"]], 
"OOP":[["oop1","av1","av2","av3"],["oop","av1","av2"]], 
"FA":["av1","av2","avN"] }

TODO cache all intermediate images.
Caching Images:
(Img, OP1 ) -> Image
(Img, OP1, OP2 ) -> Image
(Img, OP1, OP3,...OPN ) -> Image

Caching Objects:
(Img, <Image OPs>, <Object Selectors>, <ObjectOps>  )

*/

class FeatureSetResulfs
{
public:
	QList< QFutureWatcher<double>* > _inExtraction;
	QStringList _featureSet;
};

typedef QSharedPointer<FeatureSetResulfs> TPFeatureSetResults;

class FEATEX_EXPORT FeatEx : public QObject
{
    Q_OBJECT

public:
    FeatEx(QObject *parent = 0);
    ~FeatEx();

	static QStringList features() //TODO why static, why here all the methods?
	{
		return FeatureManager::instance().featureNames();
	}

	static bool exists(QString n) 
	{
		return FeatureManager::instance().exists(n);
	}

	static TPFeature getFeature( QString name )
	{
		return FeatureManager::instance().getFeature( name );
	}

	static QString getFeatureInfo( QString name )
	{
		return FeatureManager::instance().getFeatureInfo( name );
	}

	double getSampleFeatureValue( QString sampleRef, QString featureName, QList<QRect> * positions = 0 );

	QList<QRect> getCellRegions( QImage img, int min, int max, int minCells=15, int sim=0, int roi=0 );
	QList<QRect> getHomMitosisRegions( QImage img, int min, int max );

	bool extractSampleFeatures( QString sampleRef, QStringList features, bool concurrent=true );

	static QString autoFeatureName( QString fn, TFeatParams fp );

	static bool splitFeatureName( QString f, QString & outname, QString & outver, TFeatParams & params )
	{
		// TODO check string
		outname = f.section("_",0,0);
		outver = f.section("_",1,1);
		QStringList fps = f.split("_");

		fps.removeAt(0);//Remove name
		fps.removeAt(0);//Remove ver
		foreach (QString fp, fps)
		{
			params[fp.section(":",0,0)] = fp.section(":",1);
		}
		return true; 
	}

	static bool evalParams( QImage img, TFeatParams &params )
	{
		return Feature::evalParams( img, params );
	}

	static void showFeatures( bool e = true )
	{
		Feature::showFeatures(e);
	}

	void clearClassStatistics( QString );
	void clearAllStatistics( );

	bool createDatabase( QString path, bool useExisting = true );

	bool stopCalculation();
	
private:
    QSqlDatabase _database;
	bool _stop;

	QMap<QString, QImage> _usedImages;
	QMutex _usedImagesMx;
	QStringList _usedImagesQueue;
	QMap<QString, TPFeatureSetResults > _featureSets;
	QMutex _featureSetsMx;
	QMutex _dbMx;
	QMutex _deleteMx;
	int _futureCnt;
	QMap<int, QFuture<double> > _runningFutures;
	QMutex _runningFuturesMx;

private slots:
	void processExtractionFinished();	

signals:
	void sampleFeaturesExtracted( QString sampleRef, QStringList features );
	
};

#endif // FEATURE_EXTRACTOR_HG__H
