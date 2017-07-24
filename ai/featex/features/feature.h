#ifndef PATDES_FEATURES__H
#define PATDES_FEATURES__H
/*
In development module for managing and defining features.
Features are named, configurable features of a class.
The implementation of a feature is flexible. It can be considered as a Feature-class.
The concrete features are defined by setting the parameters. 
The name of the feature consists of <FeatureClass>_[<parameter_name>:<parameter_value>_...]
For example a feature-class "cell size" could have parameters such as diameter of a blob and a a threshold
for separating the blob. Many "cell size" features could be defined for extraction e.g:

CellSize_min:30_max_120_th50 - would find cells with low contrast/fluorescence intensity
CellSize_min:30_max_120_th100- 
CellSize_min:30_max_120_th200- would find only bright cells (blobs)

CellSize_min:80_max_100_th50- would find only cells within a very tight range of diameters

*/
#include <QMap>
#include <QString>
#include <QStringList>
#include <QList>
#include <QRect>
#include <QMutex>
#include <QSharedPointer>

class QImage;
class Feature;

typedef QMap<QString, QVariant> TFeatParams;
typedef std::auto_ptr<Feature> TPFeature;

class FeatureCreator
{
	QString _info;
public:
	QString info() const {return _info;}
	FeatureCreator(QString info):_info(info){}
	virtual ~FeatureCreator(){}
	virtual TPFeature create()=0;
};

#define REGISTER_FEATURE( name, classname, info ) \
class creator##classname : public FeatureCreator { public: creator##classname(QString info):FeatureCreator(info){} TPFeature create( ){ return TPFeature(new classname);} };\
	FeatureCreator * object##classname = new creator##classname(info);\
	static bool dummyInitializer##classname = FeatureManager::instance().registerFeature( #name, object##classname );

#define FEATURELIB_EXPORT

class FEATURELIB_EXPORT FeatureManager
{
	FeatureManager(){};
	QMap<QString, FeatureCreator*> _features;
public:
	static FeatureManager & instance()
	{
		static FeatureManager inst;
		return inst;
	}
	bool registerFeature( QString name, FeatureCreator*f )
	{
		_features[name] = f;
		return true;
	}	

	TPFeature getFeature( QString name )
	{
		if ( !_features.contains(name) )
			return TPFeature(0);
		return _features[name]->create();
	}

	bool exists( QString name )
	{
		return _features.contains(name);
	}

	QString getFeatureInfo( QString name ) const
	{
		if ( !_features.contains(name) )
			return QString::null;
		return _features[name]->info();
	}

	QStringList featureNames() const
	{
		return _features.keys();
	}

};

class ParamInfo
{
public:
	QString _name;
	QString _info;
	QString _constrains;
	ParamInfo(QString name=QString::null, QString info=QString::null, QString constr=QString::null):
	_name(name)
	,_info(info)
	,_constrains(constr){}
};

typedef QList<ParamInfo> TParamInfoList;

struct FeatureResults
{
	FeatureResults(double value=0.0, qint64 extrTime=0, QList<QRect> positions=QList<QRect>() )
		:_value(value),_extractionTimeMs(extrTime),_positions(positions){}
	QList<QRect> _positions;
	double _value;
	qint64 _extractionTimeMs;
};
typedef QSharedPointer<FeatureResults> TPFeatureResults;

class FEATURELIB_EXPORT Feature
{	
protected:
	TParamInfoList _paramInfos;
	double eval( QImage img, const QString & expression ) const;
	QString eval( const QString & expression ) const;

public:
	virtual ~Feature(){}
	virtual QString version()=0;
	virtual TPFeatureResults extract( QImage & img, TFeatParams params )=0;
	const TParamInfoList & parameters() const { return _paramInfos; } 
	static bool evalParams( QImage img, TFeatParams &params );
	static bool showFeatures( bool e=false , bool set=false ) 
	{ 
		static bool _showFeatures = false;
		if ( set )
			_showFeatures = e;
		return _showFeatures;
	}
};



#endif // PATDES_FEATURES__H
