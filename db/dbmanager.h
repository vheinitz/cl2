#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QList>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDir>
#include <QDateTime>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QSqlError>
#include "pddefines.h"

class CORELIB_EXPORT DBObject
{
public:
	static DBObject null;
	DBObject( QString otype=QString::null ):_type(otype){/*_["id"]=-1;*/}
	int _key;
	QString _type;
	QMap<QString, QVariant> _;
	QList<DBObject> _children;
	QString toJSON() const;
	bool contains( const QString &name)const{return _.contains(name);}
};

class CORELIB_EXPORT DbManager
{
	QStringList _transactionAddFiles; // files to be tracked during transaction. delete on rollback, clear list on commit
	bool _transactionStarted;		  // track files only if a transaction was started indeed    
    QSqlDatabase _db;
	int _lastInsertedId;
	QSqlQuery _query;    

	QMap<QString,DBObject > _prototypes;

public:

	DbManager();
	~DbManager(){ _db.close(); }
	bool check(); //TODO check if files are consistent with references
	const QString schemaHash( ) const;

	QString root();

	bool trackTransactionAddFile(QString f);
	
	bool addPrototype( DBObject p);

	bool beginTransaction();
	bool commitTransaction();
	bool rollbackTransaction();

	DBObject getObject( QString tname, int key );
	bool deleteObject(  QString otype, int key );
	bool deleteObject( DBObject );
	bool clearTable( QString otype );
	DBObject getPrototype( QString otype );
	QList<DBObject> getObjects( QString otype, const QString & sqlcondition = QString::null, const QString & orderBy = QString::null );
	QList<DBObject> getObjectsExt( QString otype, const QString & sql );
	bool addObject( DBObject & obj );
	bool updateObject( DBObject & obj );

	//QSqlQuery & query(){ return _query;}


    /*static DbManager &instance()
    {
        static DbManager inst;
        return inst;
    }
	*/

	QStringList tables();

    QSqlDatabase & db(){return _db; }
	bool exec( QString q )
	{
		_lastInsertedId=-1;
		if (!_db.open()) {
             return false;
         }  
		_query = QSqlQuery(_db);
		_db.transaction();
		bool ret = _query.exec(q);
		_db.commit();
		_lastInsertedId = _query.lastInsertId().toInt();
		return ret;
	}

	QString lastError(  )
	{
		QSqlError err = _query.lastError(); 
		return QString("DB:%1. Drv:%2").arg(err.databaseText()).arg(err.driverText());
	}

	int lastInsertedId() const { return _lastInsertedId;}

	QStringList getColumn( QString n )
	{
		 QSqlRecord rec = _query.record();

		 int col = rec.indexOf(n);
		 QStringList ret;
		 while ( col >=0 && _query.next() )
			ret << _query.value(col).toString();
		 return ret;
	}

	QStringList getColumn( int colidx )
	{
		 QSqlRecord rec = _query.record();
		 
		 QString n = rec.fieldName(colidx);
		 QStringList ret;
		 if (_query.first() && !n.isEmpty())
			do 
				ret << _query.value(colidx).toString();
			while ( _query.next() );
		 return ret;
	}

	QSqlQuery lastResult(  ) const
	{
		 return _query;
	}
	
	QString getFileDataHash( QString fileSource );
	bool checkDatabase( QString path );
	bool initDatabase( QString path );
	bool openDatabase( QString path );
 
};

#endif // DBMANAGER_H
