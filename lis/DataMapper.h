#ifndef DataMapper_H
#define DataMapper_H

#include "cldef.h"
#include "ASTMRecord.h"

#include <QObject>
#include <QMap>
#include <QSqlDatabase>
#include <QSharedPointer>
#include <QStringList>
#include <QStandardItemModel> //abusing as tree data-struct. todo change!
#include <QModelIndex>
#include <QByteArray> 
#include <QList>


class LisTraQGui;


//namespace datamapper
//{

class Instruction;
typedef QSharedPointer<Instruction> PInstruction;
typedef QList<PInstruction> TInstructions;

class CLIB_EXPORT ASTMFactory : public QObject
{
	ASTMFactory()
	{
		_recordNames[EPatient] = tr("Patient");
		_recordNames[EOrder] = tr("Order");
		_recordNames[EResult] = tr("Result");
		_recordNames[EComment] = tr("Comment");
		_recordNames[EHeader] = tr("Header");
		_recordNames[ETerminator] = tr("Terminator");
		clear();
	};
public: 
	static ASTMFactory & instance()
	{
		static ASTMFactory inst;
		return inst;
	}
	QMap<RecordType,QString> _recordNames;

	void clear()
	{		
		_records.clear();
		_records.setColumnCount(1);
	}

	void addSession( int after=-1, const QString & name=QString::null );

	void addPatient(int sessionIdx=-1)
	{
		addRecord( EPatient );
	}
	void addOrder(int sessionIdx=-1)
	{
		addRecord( EOrder );
	}
	void addResult(int sessionIdx=-1)
	{
		addRecord( EResult );
	}
	void addComment(int sessionIdx=-1)
	{
		addRecord( EComment );
	}
	void addHeader(int sessionIdx=-1)
	{
		addRecord( EHeader );
	}
	void addTerminator(int sessionIdx=-1)
	{
		addRecord( ETerminator );
	}
	void addMessage(int session=-1);

	QStandardItem * addRecord( RecordType rt );
	bool setRecord( PASTMRecord rec );

	QList<QByteArray> sessionData( int sessIdx=0 );

	bool dataFromMessage( QStandardItem *mesg, QByteArray &outData );
	bool dataFromMessage( QStandardItem *mesg, QList<QByteArray> &outData );

private:
	QStandardItemModel _records;
	QModelIndex _currentItem;
};

class CLIB_EXPORT Instruction
{	
	public: 
		virtual bool process() = 0;
		TInstructions _subInstructions;	
		//TODO: very dirty! define clean interface!!!
		QStringList _names;
		static QMap<QString, QString> _vars;
};

class CLIB_EXPORT SQLLoop : public Instruction
{		
	public: 
		SQLLoop(QString db, QString sql);
		QString _dbId;
		QString _sql;		
		virtual bool process();
};

class CLIB_EXPORT ASTMBuild : public Instruction
{	
	public: 
		ASTMBuild(QString recType, QString fields);
		QString _recType;
		QString _fields;		
		virtual bool process();
};


class CLIB_EXPORT ASTMSend : public Instruction
{		
	public: 
		ASTMSend(QString llink, QString llinkParams, QString db, QString sqlOnSuccess, QString sqlOnError );
		QString _dbId;
		QString _sqlOnSuccess;
		QString _sqlOnError;
		QString _lisLink;
		QString _lisLinkParams;
		virtual bool process();
};

class CLIB_EXPORT ASTMSendCallback:public QObject
{	
	Q_OBJECT
		QTimer *_timer;
	public: 
		ASTMSendCallback(QString db, QString sqlOnSuccess, QString sqlOnError);
		QString _dbId;
		QString _sqlOnSuccess;
		QString _sqlOnError;
		int _timeout;
		
	public slots:
		//void startTimeout(  )
		//id for having only one signaling object
		void success();
		void timeout();
		void error(QString=QString::null);
};

class CLIB_EXPORT DataMapper : public QObject
{
	Q_OBJECT
	DataMapper();
	
public:
	static DataMapper & instance()
	{
		static DataMapper inst;
		return inst;
	}
	bool processScript( QString scr );
	virtual ~DataMapper();
	QMap<QString, QSqlDatabase> _dbs;
	PInstruction _mainInstr;

	void success(QString tok);
	void failed(QString tok);


signals:
	void sigSuccess(QString);
	void sigFailed(QString);

};
//}
#endif
