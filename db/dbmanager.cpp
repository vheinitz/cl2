#include "dbmanager.h"
#include <QCryptographicHash>
#include <clog.h>

DBObject DBObject::null;
QString DBConnectionName;

DbManager::DbManager()
{
	DBConnectionName = QString("DBManager_%1").arg( (qlonglong)this );
}

QString v2j(QString n, QVariant v)
{
	return QString("\"%1\":\"%2\"").arg(n.replace("\"","\\\"")).arg(v.toString().replace("\"","\\\""));
}

QString DBObject::toJSON() const
{
	QString json;
	json += v2j("key",_key);
	json += ",";
	json += v2j("type",_type);
	if (!_.isEmpty())
		json += ",";

	int items=_.size();
	for ( QMap<QString, QVariant>::ConstIterator it=_.begin(), end=_.end(); it!=end;++it )
	{
		json += v2j(it.key(),it.value());
		if( --items>0 )
			json += ",";
	}
	return QString("{%1}").arg(json);
}

bool DbManager::clearTable( QString tname )
{
	QString sql("DELETE ");
	sql +=" FROM ";
	sql +=tname;

	QSqlQuery query(_db);
	bool ok = query.exec(sql);
	if (!ok)
	{
		C_ERROR("Can't clear table %s", QS2CS(tname) );
		return false;
	}

	return true;
}

bool DbManager::trackTransactionAddFile(QString f)
{
	if( _transactionStarted )
	{
		_transactionAddFiles.append(f);
	}
	return QFileInfo(f).exists();
}

bool DbManager::addPrototype( DBObject p)
{
	_prototypes[p._type] = p;
	return true;
}

bool DbManager::beginTransaction()
{
	QString sql("BEGIN TRANSACTION ");	
	QSqlQuery query(_db);
	bool result = query.exec(sql);
	if (result)
	{
		_transactionStarted=true;
	}
	return result;
}

bool DbManager::commitTransaction()
{
	QString sql("COMMIT TRANSACTION ");	
	QSqlQuery query(_db);
	bool result = query.exec(sql);
	_transactionAddFiles.clear();
	_transactionStarted=false;
	return result;
}

bool DbManager::rollbackTransaction()
{
	QString sql("ROLLBACK TRANSACTION ");	
	QSqlQuery query(_db);
	bool result = query.exec(sql);

	foreach( QString f, _transactionAddFiles )
	{
		result &= QFile::remove(f);
	}
	_transactionAddFiles.clear();
	_transactionStarted=false;

	return result;
}


DBObject DbManager::getObject( QString otype, int key )
{
	if (!_prototypes.contains(otype))
	{
		C_ERROR("No such DB-object %s", QS2CS(otype) );
		return DBObject::null;
	}

	DBObject dbobj = _prototypes[otype];
	QString cols(" id, " );
	QStringList names = dbobj._.keys();
	cols += names.join(", ");
	cols+=" ";

	QString sql("SELECT ");
	sql +=cols;
	sql +=" from ";
	sql +=otype;
	sql +=" WHERE id=";
	sql += QString::number(key);
	
	QSqlQuery query(_db);
	bool ok = query.exec(sql);
	if (!ok)
	{
		C_ERROR("DB-object %s not found at row %d", QS2CS(otype), key );
		return DBObject::null;
	}

	DBObject dbo(otype);
	if( query.next() )
	{
		int i=1;
		foreach (QString n, names)
		{
			dbo._[n] = query.value(i++).toString();
		}
		dbo._key = query.value(0).toInt();
	}
	return dbo;
}

QList<DBObject> DbManager::getObjectsExt( QString otype, const QString & extsql )
{
	QList<DBObject> ret;
	if (!_prototypes.contains(otype))
	{
		C_ERROR("No such DB-object %s", QS2CS(otype) );
		return ret;
	}

	DBObject dbobj = _prototypes[otype];
	QString cols = QString(" %1.id, " ).arg(otype);
	QStringList names = dbobj._.keys();
	QStringList fullNames;
	foreach( QString n, names )
		fullNames<<QString("[%1].%2").arg(otype).arg(n);


	cols += fullNames.join(", ");
	cols+=" ";

	QString sql("SELECT ");
	sql +=cols;
	sql +=" FROM ";
	sql +=otype;
	sql += " ";
	sql += extsql;

	
	QSqlQuery query(_db);
	bool ok = query.exec(sql);
	if (!ok)
	{
		C_ERROR("Invalid SQL \"%s\"", QS2CS(sql) );
		return ret;
	}

	DBObject dbo(otype);
	while( query.next() )
	{
		int i=1;
		foreach (QString n, names)
		{
			dbo._[n] = query.value(i++).toString();
		}
		dbo._key = query.value(0).toInt();
		ret.append(dbo);
	}
	return ret;

}


DBObject DbManager::getPrototype( QString otype )
{
	if (!_prototypes.contains(otype))
	{
		C_ERROR("No such DB-table %s", QS2CS(otype) );
		return DBObject();
	}

	return _prototypes[otype];
}

//TODO: C&P above
QList<DBObject> DbManager::getObjects( QString otype, const QString & sqlcondition, const QString & orderBy )
{
	QList<DBObject> ret;
	if (!_prototypes.contains(otype))
	{
		C_ERROR("No such DB-object %s", QS2CS(otype) );
		return ret;
	}

	DBObject dbobj = _prototypes[otype];
	
	QStringList names;
	names << "id";
	names.append (dbobj._.keys());
	QString cols = names.join(", ");
	cols+=" ";

	QString sql("SELECT ");
	sql +=cols;
	sql +=" from ";
	sql +=otype;
	if ( !sqlcondition.isNull() && !sqlcondition.simplified().isEmpty()  ) 
	{
		sql +=" WHERE ";
		sql += sqlcondition;
	}

	if ( !orderBy.isNull() && !orderBy.simplified().isEmpty() )
	{
		sql +=" ORDER BY ";
		sql += orderBy;
	}
	
	QSqlQuery query(_db);
	bool ok = query.exec(sql);
	QSqlError err = query.lastError();
	if (!ok)
	{
		C_ERROR("Invalid SQL \"%s\"", QS2CS(sql) );
		return ret;
	}

	DBObject dbo(otype);
	while( query.next() )
	{
		int i=0;
		foreach (QString n, names)
		{
			dbo._[n] = query.value(i++).toString();
		}
		dbo._key = query.value(0).toInt();
		ret.append(dbo);
	}
	return ret;


}

bool DbManager::addObject( DBObject & dbo )
{
	if (!_prototypes.contains(dbo._type))
	{
		C_ERROR("Can't add object '%s' without prototype", QS2CS(dbo._type) );
		return false;
	}

	DBObject dbobj = _prototypes[dbo._type];
	QString cols;
	QString binders;
	QStringList names = dbobj._.keys();
	cols += names.join(", ");
	foreach (QString bn, names )
	{
		if (!binders.isEmpty())
			binders +=", ";

		binders += QString(":%1").arg(bn);
	}


	QString sql("INSERT INTO ");
	sql +=dbo._type;
	sql +="( ";
	sql +=cols;
	sql +=" )";
	

	sql +=" VALUES (";
	sql +=binders;
	sql +=" )";
	
	QSqlQuery query(_db);
	query.prepare( sql );
	//QList<QVariant> vals = dbo._.values();
	QString svals;
	foreach (QString n, names )
	{
		query.bindValue( QString( ":%1" ).arg(n), dbo._[n] );
		//if (!svals.isEmpty())
		//	svals +=", ";
		//svals+=QString("\"%1\"").arg( dbo._[k].toString() );

	}
	bool ok = query.exec();
	if (!ok)
	{
		C_ERROR("Invalid SQL \"%s\"", QS2CS(sql) );
		return false;
	}

	_lastInsertedId = query.lastInsertId().toInt();
	dbo._key = _lastInsertedId ;

	return true;
}

bool DbManager::deleteObject( QString otype, int key )
{
	if (!_prototypes.contains(otype))
	{
		C_ERROR("Can't delete object '%s' without prototype", QS2CS(otype) );
		return false;
	}

	QString sql("DELETE FROM ");
	sql += otype;
	sql += " WHERE id=";
	sql += QString::number(key );
	
	QSqlQuery query(sql, _db);

	bool ok = query.exec();
	if (!ok)
	{
		C_ERROR("Invalid SQL \"%s\"", QS2CS(sql) );	
		return false;
	}

	return true;
}

bool DbManager::deleteObject( DBObject filterObject )
{
	if (!_prototypes.contains(filterObject._type))
	{
		C_ERROR("Can't update object '%s' without prototype", QS2CS(filterObject._type) );
		return false;
	}

	DBObject dbobj = _prototypes[filterObject._type];

	QString sql("DELETE FROM ");
	sql +=filterObject._type;

	QString svals;
	for ( QMap<QString,QVariant>::iterator it=filterObject._.begin(); it!=filterObject._.end(); ++it )
	{
		if (!svals.isEmpty())
			svals +=", ";		
		svals += QString("%1=:%1").arg(it.key());
	}
	
	sql += svals;
	sql += " WHERE ";
	sql += svals;
	
	QSqlQuery query(_db);
	query.prepare(sql);

	for ( QMap<QString,QVariant>::iterator it=filterObject._.begin(); it!=filterObject._.end(); ++it )
	{
		query.bindValue( QString(":%1").arg(it.key()), it.value() );
	}

	bool ok = query.exec();
	if (!ok)
	{
		C_ERROR("Invalid SQL \"%s\"", QS2CS(sql) );	
		return false;
	}

	return true;
}

bool DbManager::updateObject( DBObject & dbo )
{
	if (!_prototypes.contains(dbo._type))
	{
		C_ERROR("Can't update object '%s' without prototype", QS2CS(dbo._type) );
		return false;
	}

	DBObject dbobj = _prototypes[dbo._type];

	QString sql("UPDATE ");
	sql +=dbo._type;
	sql +=" SET ";

	QString svals;
	for ( QMap<QString,QVariant>::iterator it=dbo._.begin(); it!=dbo._.end(); ++it )
	{
		if (!svals.isEmpty())
			svals +=", ";		
		svals += QString("%1=:%1").arg(it.key());
	}
	
	sql += svals;
	sql += " WHERE id=";
	sql += QString::number( dbo._key );
	
	QSqlQuery query(_db);
	query.prepare(sql);

	for ( QMap<QString,QVariant>::iterator it=dbo._.begin(); it!=dbo._.end(); ++it )
	{
		query.bindValue( QString(":%1").arg(it.key()), it.value() );
	}

	bool ok = query.exec();
	if (!ok)
	{
		C_ERROR("Invalid SQL \"%s\"", QS2CS(sql) );	
		return false;
	}

	dbo._key = query.lastInsertId().toInt();

	return true;
}

QString DbManager::getFileDataHash( QString fileSource )
{
	static QMap<QString, QString> _cache;
	if (_cache.contains(QFileInfo(fileSource).canonicalFilePath()))
		return _cache[QFileInfo(fileSource).canonicalFilePath()];

	QFile f( fileSource );
	if (!f.open(QIODevice::ReadOnly))
		return QString::null;

	QString hash = QString(QCryptographicHash::hash( f.readAll() ,QCryptographicHash::Md5).toHex());
	_cache[QFileInfo(fileSource).canonicalFilePath()] = hash;
	return hash;
}


bool DbManager::openDatabase( QString path )
{
	if (!QFileInfo(path).exists())
		return false;

	_db = QSqlDatabase::addDatabase("QSQLITE", DBConnectionName);
	_db.setDatabaseName( path );
	/*POSTGRES
	_db = QSqlDatabase::addDatabase("QPSQL", DBConnectionName);
	_db.setHostName("localhost");
	_db.setPort(5432);
	_db.setDatabaseName("mydbname");
	_db.setUserName("postgres");
	_db.setPassword("test");
	*/
	if (!_db.open())
		return false;

	_prototypes.clear();
	_transactionAddFiles.clear();
	_transactionStarted = 0;
	_lastInsertedId=-1;

	_query = QSqlQuery( "SELECT name FROM sqlite_master WHERE NOT name LIKE 'sqlite_%' ",_db );

	while( _query.next() )
	{
		QString tab = _query.value(0).toString();
		DBObject table( tab );
		_prototypes[tab] = table;
	}

	foreach( QString t, _prototypes.keys() )
	{
		_query = QSqlQuery( QString("select * from %1 LIMIT 0 ").arg(t), _db );
		QSqlRecord rec = _query.record();

		for(int i=0; i< rec.count(); ++i)
		{
			if(rec.fieldName(i) != "id") //TODO use id as a generic col name in prototypes
				_prototypes[t]._[rec.fieldName(i)];
		}
	}

    return true;
}

/**
Checks the database.
There could be 3 cases:
 1)Database is available and schema valid - return ok
 2)Database is available but schema not valid - try to rename and re-create new
 3)Connecting or creating fails - return error
*/
bool DbManager::checkDatabase( QString path )
{
	if( !QFileInfo(path).exists() )
	{
		C_ERROR( "No Database %s", QS2CS( path ) );
		return false;
	}
	
	_db = QSqlDatabase::addDatabase("QSQLITE", DBConnectionName);
	_db.setDatabaseName( path );
	_db.open();
/*	{
		C_ERROR( "Can't open Database %s", QS2CS( path ) );
		return false;
	}
*/
	
	QString sql = QString("SELECT meta_information_value FROM db_meta_information WHERE meta_information_key='schema_version' ");
	QSqlQuery query(_db);
	if ( !query.exec(sql) )                 //old schema because the table db_meta_information doesn't exist
	{

		C_ERROR( "Invalid schema on %s", QS2CS( path ) );
		return false;
	}
	else if ( query.record().count() != 1 ) //old schema because meta_information_value has no/redundant value/s
	{
		C_ERROR( "Invalid or redundant schema-version on %s", QS2CS( path ) );
		return false;
	}
									  
	query.next();
	QString sh_stored = query.value(0).toString();
	QString sh_req = schemaHash();
	if ( sh_stored != sh_req )
	{
		C_ERROR( "Invalid schema-version on %s", QS2CS( path ) );
		return false;                       //old schema because meta_information_value has wrong value
	}

	return true;
}

/**
Initializes the database.
DB-file shouldn't exist!
*/
bool DbManager::initDatabase( QString path )
{
		
	bool ok=true;

	_db = QSqlDatabase::addDatabase("QSQLITE", DBConnectionName);
	_db.setDatabaseName( path );
	_db.open();
	/*{
		C_ERROR( "Can't open Database %s", QS2CS( path ) );
		return false;
	}*/

	QStringList tables = _prototypes.keys();
	QSqlQuery query(_db);
	foreach( QString tab, tables )
	{
		DBObject dbobj = _prototypes[tab];
		QString cols;
		QString colNames;
		for ( QMap<QString, QVariant>::iterator it = dbobj._.begin();
				it != dbobj._.end();
				++it)
		{
			if(!colNames.isEmpty())
				colNames += ", ";
			colNames += it.key();
		}
							

		// Create 
		QString sql = QString("CREATE TABLE ");
		sql += tab + "(\n";
		
		//SQLITE 
		sql += "id INTEGER PRIMARY KEY AUTOINCREMENT,\n";

		cols.clear();
		for ( QMap<QString, QVariant>::iterator it = dbobj._.begin();
				it != dbobj._.end();
				++it)
		{
			if(!cols.isEmpty())
				cols += ",\n";

			QString dataType = "TEXT";
			if ( it.value().type() == QVariant::Int )
			{
				dataType = "INTEGER";
			}
			else if ( it.value().type() == QVariant::Double )
			{
				dataType = "REAL";
			}

			cols += it.key() + " " + dataType;
		}
		sql+=cols;
		sql+=")";
		ok = query.exec(sql);
		if(!ok)
		{
			C_ERROR("Invalid SQL \"%s\"", QS2CS(sql) );
			ok=false;
		}
	}
	DBObject dbobj = _prototypes["db_meta_information"];
	dbobj._["meta_information_key"] = "schema_version";
	QString sh = schemaHash();
	dbobj._["meta_information_value"] = sh;
	ok &= addObject(dbobj);

	return ok;
}



const QString DbManager::schemaHash( ) const
{
	QStringList tables = _prototypes.keys();
	QString dbStrucureData = tables.join("\n");

	foreach( QString tab, tables )
	{
		DBObject dbobj = _prototypes[tab];
		for ( QMap<QString, QVariant>::iterator it = dbobj._.begin();
				it != dbobj._.end();
				++it)
		{		
			dbStrucureData += it.key();
			dbStrucureData +="\n";
		}
	}

	QString hash = QCryptographicHash::hash( dbStrucureData.toLatin1() , QCryptographicHash::Md5 ).toHex();
	return hash;
 }