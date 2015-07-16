/* ===========================================================================
 * Copyright 2015: Valentin Heinitz, www.heinitz-it.de
 * Key-value store with publisher/subscriber abilities
 * Author: Valentin Heinitz, 2015-04-12
 * License: MIT
 *
 ========================================================================== */


#ifndef KEY_VALUE_STORE_HG
#define KEY_VALUE_STORE_HG

#include <QObject>
#include <QMap>
#include <set>
#include <QSet>
#include <QVariant>
#include <QStringList>
#include <QTimer>
#include <QQueue>
 
#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

static const bool DontStoreInThisFile=false; //named false
static const bool StoreInThisFile=true; //named true
static const bool UpdateNow=true; //named true

//---------- Data Structure for setting values asynchronously. 
//Introduced for avoiding SO in QC test run
class CLIB_EXPORT AsyncSetObject
{    
public:
    QString _pvName;
	QVariant _val;
	QObject *_sender;
	AsyncSetObject( QString pv=QString::null, QVariant val=QVariant(), QObject *s=0 ):
		_pvName(pv),
		_val(val),
		_sender(s)		
		{}
};
typedef QQueue<AsyncSetObject> TAsyncSetQueue;
//---------- END Data Structure for setting values asynchronously

//TODO add constraints here also
class CLIB_EXPORT PDReceiverObject
{
public:
    QObject * _instance;
    QString _propertyName;
    PDReceiverObject( QObject *inst, QString prop ):_instance(inst),_propertyName(prop){}
    bool operator<( const PDReceiverObject & other ) const
    {
        return _instance < other._instance;
    }
};

typedef std::set<PDReceiverObject> TValueReceivers;


enum ProcessDataType
{
    PDT_In,
    PDT_Out,
    PDT_InOut,
};

class ProcessDataObject
{
    QVariant _value;	
	int _changed; ///<<used for persistence, profiling and coverage
	mutable int _used; ///<<used for profiling and coverage, should be used in debug mode only

public:
    TValueReceivers _receivers;
    int _dataIdx;
    QString _info;
    ProcessDataType _type;
    QVariant _min;
    QVariant _max;
	QVariant _defaultValue;
	bool _isPersistent;
	
    ProcessDataObject();

	///Sets value without notifying connected receivers
	void setInitialValue( QVariant v );

	///Sets default value 
	void setDefaultValue( QVariant v );

	const QVariant & defaultValue( ) const;
	void resetToDefault( QVariant v );
    void setValue( QVariant v );
    const QVariant & value( ) const;
	int changed( ) const;
	int used( ) const;
	void resetChanged( ){ _changed=false; }
};



class CLIB_EXPORT KVStore : public QObject
{
    Q_OBJECT

private:    
    typedef QMap<QString, ProcessDataObject> TProcessData;
    TProcessData _kvstore;
    QSet<QObject*> _receivers;
    bool _ignoreUiChanges;
    void setDataIdx(QString var, int idx );
    QString _source;
	QString _persistence;
	QString _dataRootPath;
	TAsyncSetQueue _asyncSetQueue;

private slots:
	void processSetAsynchronously();

public:
    KVStore(QObject *p=0);    
    ~KVStore();
	void reset( );
	void resetChanged( bool onlyPersistent=true );
    bool changed( bool onlyPersistent, QString & firstChanged, QStringList ignoreList = QStringList() );
	void setDataRoot( const QString & path );
	QStringList vars( ) const { return _kvstore.keys(); }
	QMap<QString, QVariant> configs( ) const;
	const QString & dataRoot( ) const;

    void load( QString );

	void loadPersistence( const QString & file, bool updateNow=true );

	void storePersistence( );

    QVariant get(QString) const;
	QString sget(QString) const;
	int iget(QString) const;
    QString getInfo(QString) const;
	QVariant getDefaultValue(QString name) const;
    bool set (QString var, QVariant val=QVariant(), QObject* sender=0);
	bool setValue (QString var, QVariant val);
	void aset (QString var, QVariant val=QVariant(), QObject* sender=0);
	bool create (QString var); //used for unit outputs
	bool varExists (QString var) const { return _kvstore.contains( var ); } 
    void subscribe( QString var, QObject*, QString propertyName=QString::null, bool create=false);

    //void unsubscribe( var, QObject* ); todo: implement
    //void unsubscribe( var, QObject*, prop ); todo: implement
public slots:
    void unsubscribe( QObject* ); // for unsubscribing on object destroyed
signals:
    void valueChanged( QString var, QVariant val );


};

#endif // PDDATAPOOL_H
