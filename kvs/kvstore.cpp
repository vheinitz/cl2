#include "kvstore.h"
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QSet>
#include <QFileInfo>

	
ProcessDataObject::ProcessDataObject():_changed(0), _used(0),_dataIdx(0),_type( PDT_InOut ), _isPersistent(false){}

void ProcessDataObject::setInitialValue( QVariant v )
{		
    _value = v;
}

void ProcessDataObject::setDefaultValue( QVariant v )
{		
	_defaultValue = v;
}

const QVariant & ProcessDataObject::defaultValue( ) const
{		
	return _defaultValue;
}

void ProcessDataObject::resetToDefault( QVariant v )
{		
	_defaultValue = v;
}

void ProcessDataObject::setValue( QVariant v )
{
	if ( _value != v )
		++_changed;
    _value = v;	
	++_used;
}
const QVariant & ProcessDataObject::value( ) const
{
    ++_used;
	return _value;		
}
int ProcessDataObject::changed( ) const
{
    return _changed;
}
int ProcessDataObject::used( ) const
{
    return _used;
}

KVStore::KVStore(QObject *p) : QObject(p)
{    
}

KVStore::~KVStore()
{
}

QMap<QString, QVariant> KVStore::configs( ) const
{
	QMap<QString, QVariant> cm;

	for( TProcessData::const_iterator it = _kvstore.begin(), end = _kvstore.end(); it!= end; ++it )
	{
		if(it.value()._isPersistent)
			cm[it.key()] = it.value().value();
	}

	return cm;
}

void KVStore::resetChanged( bool onlyPersistent )
{
	for( TProcessData::iterator it = _kvstore.begin(); it != _kvstore.end(); ++it )
	{
		if(onlyPersistent && it.value()._isPersistent)
			it.value().resetChanged();
		else
			it.value().resetChanged();
	}
}

bool KVStore::changed( bool onlyPersistent, QString & firstChanged, QStringList ignoreList )
{

	for( TProcessData::const_iterator it = _kvstore.begin(), end = _kvstore.end(); it!= end; ++it )
	{
		if(onlyPersistent)
		{
			if( it.value()._isPersistent && it.value().changed() && !ignoreList.contains( it.key() ) )
			{
				firstChanged = it.key();
				return true;
			}
		}
		else
		{
			if( it.value().changed() && !ignoreList.contains( it.key() ) )
			{
				firstChanged = it.key();
				return true;
			}
		}		
	}
	return false;
}

void KVStore::loadPersistence( const QString & fname, bool updateNow )
{
	_persistence = fname;

	QStringList cfglines;
    QFile f(_persistence);
    if ( !f.open(QIODevice::ReadOnly) )
	{
		return;
	}

    QTextStream ts(&f);
    while (!ts.atEnd())
    {
		QString line = ts.readLine().trimmed();
		if ( line.isEmpty() || line.at(0) == '#'  ) continue;
        cfglines.append(line);
    }

    foreach(QString l, cfglines )
    {
        QString key = l.section("=",0,0).trimmed();
		QString value = l.section("=",1).trimmed();
        if ( !key.isEmpty() && l.contains('=') )
        { 
			TProcessData::Iterator it = _kvstore.find(key);
			if( it != _kvstore.end() && it->_isPersistent )
			{
				if ( !updateNow )
					it->setInitialValue( value );
				else
					set( key, value );

				it->setDefaultValue( value );
				for (TValueReceivers::iterator rit = it->_receivers.begin(); rit != it->_receivers.end(); ++rit )
				{
					rit->_instance->setProperty( "ci_stdValue", value );
				}
			}
			else
			{
//err
			}
        }
		else
		{
//err
		}
    }	
}

void KVStore::storePersistence( )
{
	QFile f(_persistence);
	if( !f.open(QIODevice::WriteOnly) )
	{
//err
		return;
	}

	QTextStream ts(&f);
	for ( TProcessData::Iterator it = _kvstore.begin(); it != _kvstore.end(); ++it)
	{
		if ( it->_isPersistent )
		{			
			if(!it.value().value().toString().isEmpty())
			{
				ts << it.key();
				ts << "=";
				ts << it.value().value().toString() <<"\n";
			}
		}
	}	
	f.close();
}

void KVStore::reset( )
{
	_dataRootPath.clear();	
	_kvstore.clear();
	_receivers.clear();
}

void KVStore::setDataRoot( const QString & path )
{
	_dataRootPath = path;	
}

const QString & KVStore::dataRoot( ) const
{
	return _dataRootPath;
}

void KVStore::load( QString fname )
{
    _source = fname;
    _kvstore.clear();

    QDomDocument domdoc;
    QString errorMsg;
    int line=0, col=0;
    QFile sourceFile ( fname );
    domdoc.setContent(&sourceFile, false, &errorMsg, &line, &col);
    if (!errorMsg.isEmpty() ) 
	{
		return;
	}
    
    QDomNodeList slideInfos = domdoc.elementsByTagName("ProcessData").at(0).toElement().elementsByTagName("ProcessDataItem");

    for (int i=0; i<slideInfos.count(); ++i)
    {
        QDomElement el  = slideInfos.at(i).toElement();
        QString name    = el.elementsByTagName( "Name" ).at(0).toElement().text();
        QString info    = el.elementsByTagName( "Info" ).at(0).toElement().text();
        QString value   = el.elementsByTagName( "Value" ).at(0).toElement().text();
        QString min     = el.elementsByTagName( "Min" ).at(0).toElement().text();
        QString max     = el.elementsByTagName( "Max" ).at(0).toElement().text();
        QString type    = el.elementsByTagName( "Type" ).at(0).toElement().text();
		bool persistent = el.elementsByTagName( "Persistent" ).at(0).toElement().text().toInt();
        
		_kvstore[ name ].setDefaultValue( value );
		_kvstore[ name ].setInitialValue( value );
        _kvstore[ name ]._info = info;
		_kvstore[ name ]._isPersistent = persistent;

        if ( type == "In" ) _kvstore[ name ]._type = PDT_In;
        else if ( type == "Out" ) _kvstore[ name ]._type = PDT_Out;
        else _kvstore[ name ]._type = PDT_InOut;

        _kvstore[ name ]._min = min;
        _kvstore[ name ]._max = max;
    }
}

QString KVStore::sget(QString name) const
{
	return get(name).toString();
}

int KVStore::iget(QString name) const
{
	return get(name).toInt();
}

QVariant KVStore::get(QString name) const
{    

    TProcessData::const_iterator i = _kvstore.find(name);
    if ( i == _kvstore.end() )
    {
        return QVariant();
    }
    QVariant val  = i.value().value();
    return  val;
}

QString KVStore::getInfo(QString name) const
{
    TProcessData::const_iterator i = _kvstore.find(name);
    if ( i == _kvstore.end() )
    {
        return QString::null;
    }
    return  i.value()._info;
}

QVariant KVStore::getDefaultValue(QString name) const
{
    TProcessData::const_iterator i = _kvstore.find(name);
    if ( i == _kvstore.end() )
    {
        return QVariant();
    }
    QVariant val  = i.value().defaultValue();
    return  val;
}

bool KVStore::create (QString name)
{
	TProcessData::iterator i = _kvstore.find(name);
    if ( i != _kvstore.end())
    {
        return false;
    }
	_kvstore[name];
	return true;

}

void KVStore::aset(QString name, QVariant value, QObject* sender)
{
	_asyncSetQueue.enqueue( AsyncSetObject(name, value, sender)  );
	QTimer::singleShot(1,this, SLOT(processSetAsynchronously()));
}

void KVStore::processSetAsynchronously()
{
	if( _asyncSetQueue.isEmpty() )
		return;

	AsyncSetObject data = _asyncSetQueue.dequeue();
	set( data._pvName, data._val, data._sender );
}

bool KVStore::set(QString name, QVariant value, QObject* sender)
{
    TProcessData::iterator i = _kvstore.find(name);
    if ( i == _kvstore.end())
    {
        return false;
    }
    else
    {
        i.value().setValue(value);
        foreach( PDReceiverObject rec, _kvstore[name]._receivers )
        {
            if( sender != rec._instance )
            {
                if ( rec._propertyName == "valueChanged" )//todo: not depending on property name
                {
                    rec._instance->setProperty(rec._propertyName.toAscii().constData(), name);
                }
                else
                {					
                    rec._instance->setProperty(rec._propertyName.toAscii().constData(), value);
                }                
            }
        }
        emit valueChanged( name, value );
    }
    return true;
}

bool KVStore::setValue(QString name, QVariant value)
{
    TProcessData::iterator i = _kvstore.find(name);
    if ( i == _kvstore.end())
    {
        return false;
    }
    
	i.value().setValue(value);    
    return true;
}


void KVStore::subscribe( 
                                  QString name, 
                                  QObject* receiver, 
                                  QString propertyName, 
                                  bool create )
{
    if ( !create && _kvstore.find(name) == _kvstore.end())
    {
		return;
    }
    else
    { 
		if(receiver)
		{
			_kvstore[name]._receivers.insert(PDReceiverObject(receiver,propertyName));
			_receivers.insert( receiver );
			connect(receiver, SIGNAL(destroyed(QObject*)), this, SLOT( unsubscribe(QObject*) ) );			
			int val = _kvstore[name].value().toInt();			
			receiver->setProperty( propertyName.toAscii().constData(), _kvstore[name].value() );
		}
		else
		{
			_kvstore[name]; // just create
		}
    }
}

void KVStore::unsubscribe( QObject* receiver )
{    
    for(TProcessData::iterator i = _kvstore.begin(); i != _kvstore.end(); ++i )
    {
        TValueReceivers::iterator ri = i.value()._receivers.begin();
        while ( ri != i.value()._receivers.end() )
        {
            if ( ri->_instance == receiver )
            {
                i.value()._receivers.erase(ri);
                ri = i.value()._receivers.begin();
                continue;
            }
            ++ri;
        }
    }
    _receivers.remove( receiver );
    //todo: remove data-objects containing no receiver 
}
