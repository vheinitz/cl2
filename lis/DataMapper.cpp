
#include "DataMapper.h"
#include <QStringList>
#include <QRegExp>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMap>
#include <QSet>
#include <QVariant>
#include <QTimer>
#include "LisLink.h"
#include <kvs/kvstore.h>
#include <log/clog.h>
#include <QTcpSocket>
#include <QFileInfo>



//using namespace datamapper;

QMap<QString, QString> Instruction::_vars;

QString DMConnectionName = "DataMapper_01";

QList<QByteArray> ASTMFactory::sessionData(int sessIdx)
{
	//TODO, find proper session
	QList<QByteArray> dataToSendNorm;
	while( _currentItem.isValid() && _currentItem.data(Qt::UserRole+1).toInt() != ESession)
						_currentItem = _currentItem.parent();
	if ( _currentItem.isValid() )  
	{
		QList<QByteArray> dataToSend;
		
		if ( _currentItem.data(Qt::UserRole+1).toInt() == ESession)
		{
			QStandardItem *msg = _records.itemFromIndex(_currentItem);
			
			
			if ( dataFromMessage( msg, dataToSend ) )
			{
				//if( !_dataSending)//todo append session
				{
					
					int seq=1;				
					for (int i =0; i<dataToSend.size(); ++i)
					{
						QByteArray tmpdata = dataToSend.at(i);
						if (tmpdata.isEmpty())
							continue;
						
						int sidx=0;
						int eidx = 0; //tmpdata.size( ) > sidx+239 ? sidx+239 : tmpdata.size( )-1;
						do
						{
							eidx = tmpdata.size( );
							if( eidx > sidx+239 )
								eidx = sidx+239;

							QByteArray data;
							
							data.append( (char)('0'+seq) );
							data.append(tmpdata.mid( sidx, eidx-sidx ));
							sidx = eidx;
							data.append(ASTM_CR);
							if( data.size()<241 )
								data.append(ASTM_ETX);
							else
								data.append(ASTM_ETB);

							
							
							unsigned char cs = Helpers::checkSum(data);
							data.append( Helpers::binToHex( cs ) );	
							data.append( ASTM_CR );
							data.append( ASTM_LF );
							data.prepend( ASTM_STX );
							dataToSendNorm.append(data);
							seq++;
							if ( seq>7 )
								seq=0;
							

						}while( eidx < tmpdata.size( )-1 );
					}
					dataToSendNorm.prepend(  QByteArray( (const char *)&ASTM_ENQ, 1 )  );
					dataToSendNorm.append(  QByteArray( (const char *)&ASTM_EOT, 1 )  );
				}
			}
		}
		else
		{
			//todo error		
		}
	}
	return dataToSendNorm;
}

bool ASTMFactory::setRecord( PASTMRecord rec )
{
	if(rec.isNull())
		return false;

	RecordType rt = rec->_type;
	QStandardItem * cur = addRecord( rt );
	if(!cur)
		return false;

	cur->setData( rec->values(), Qt::UserRole+2);

	return true;
}

QStandardItem * ASTMFactory::addRecord( RecordType rt )
{
	QStandardItem *cur = 0;
	if ( _currentItem.isValid() )  
	{
		RecordType currentDataType = (RecordType)_currentItem.data(Qt::UserRole+1).toInt();
		if ( currentDataType == EMessage)
		{
			cur = _records.itemFromIndex(_currentItem);
		}
		else if ( QString("HOPRCL").contains( (char)currentDataType )  )
		{
			switch( rt )
			{
				case EHeader:
				case EPatient:
				case ETerminator:
					while( _currentItem.isValid() && _currentItem.data(Qt::UserRole+1).toInt() != EMessage)
						_currentItem = _currentItem.parent();
					break;
				case EOrder:
					while( _currentItem.isValid() && _currentItem.data(Qt::UserRole+1).toInt() != EPatient)
						_currentItem = _currentItem.parent();
					break;
				case EResult:
					while( _currentItem.isValid() && _currentItem.data(Qt::UserRole+1).toInt() != EOrder)
						_currentItem = _currentItem.parent();
					break;
				case EComment:
					while( _currentItem.isValid() && _currentItem.data(Qt::UserRole+1).toInt() == EComment)
						_currentItem = _currentItem.parent();
					break;
			}
			cur = _records.itemFromIndex(_currentItem);
		}
		else
		{
			C_ERROR("Internal error in ASTM message builder.");
		}

		if( cur )
		{
			QStandardItem *child = new QStandardItem(ASTMFactory::instance()._recordNames[rt]);
			child->setData( rt, Qt::UserRole+1 );
			child->setEditable(false);
			cur->appendRow(child);
			_currentItem = _records.indexFromItem(child);
			cur = child;
		}
	}
	return cur;
}


void ASTMFactory::addSession( int after, const QString & name )
{	
	int rc = _records.rowCount();
	_records.insertRow( rc );
	rc = _records.rowCount();
	int curRow = rc -1;
	bool ok = _records.setData( _records.index( curRow,0), ESession, Qt::UserRole+1 );
	ok &= _records.setData( _records.index( curRow,0), (name==QString::null?QString("Session"):name) );
	_currentItem = _records.index( curRow,0);
}

void ASTMFactory::addMessage(int session)
{
	if (session==-1)
		_currentItem = _records.index(0,0);

	if (_currentItem.isValid() && _currentItem.data(Qt::UserRole+1).toInt() == ESession)
	{
		QStandardItem *child = new QStandardItem("Message");
		QStandardItem *cur = _records.itemFromIndex(_currentItem);
		child->setData( EMessage, Qt::UserRole+1 );
		child->setEditable(false);
		cur->appendRow(child);
		_currentItem = _records.indexFromItem(child);
	}
}

bool ASTMFactory::dataFromMessage( QStandardItem *mesg, QByteArray &outData )
{
	QList<QByteArray> outDataArray;
	if (!dataFromMessage( mesg, outDataArray ))
		return false;

	for (int i=0; i< outDataArray.size(); ++i)
	{
		outData.append( outDataArray.at(i) );
	}

	return true;
}

bool ASTMFactory::dataFromMessage( QStandardItem *mesg, QList<QByteArray> &outData )
{
	QStandardItem *cur = mesg;
	int curr=0;

	Separators sep;
	int patientNum=1;
	int orderNum=1;
	int resultNum=1;
	int commentNum=1;
	
	
	QList< QPair<QStandardItem *, int>  > hOrder;
	
	hOrder.append(QPair<QStandardItem *, int>(cur,curr) );
	for ( cur = hOrder.last().first->child( curr++,0); hOrder.size()>0; cur = hOrder.last().first->child( curr++,0) )
	{
		if (cur==0)
		{
			cur = hOrder.last().first;
			curr=hOrder.last().second;
			hOrder.takeLast();
			if(hOrder.size()>0)
				continue;
			else
				break;
		}
		RecordType recType = static_cast<RecordType>(cur->data( Qt::UserRole+1 ).toInt());

		ASTMRecord * rec=0;
		switch(recType)
		{
		case EHeader:
			rec = new ASTMHeader("");
			patientNum=1;
			orderNum=1;
			resultNum=1;
			commentNum=1;
			break;
		case EPatient:
			rec = new ASTMPatient(patientNum);
			patientNum++;
			orderNum=1;
			resultNum=1;
			commentNum=1;
			break;
		case EOrder:
			rec = new ASTMOrder(orderNum);
			orderNum++;
			resultNum=1;
			commentNum=1;
			break;
		case EResult:
			rec = new ASTMResult(resultNum);
			resultNum++;
			commentNum=1;
			break;
		case EComment:
			rec = new ASTMComment(commentNum);
			commentNum++;
			break;
		case ETerminator:
			rec = new ASTMTerminator(1); //todo: could be something else then 1?
			break;	
		case EMessage:
			rec = new ASTMRecord(); //todo: handle fake record message
			break;	
		default:
			return false;
		}
		//rec->_sep = sep;
		QMap<QString, QVariant> fields = cur->data( Qt::UserRole+2 ).toMap();
		for(QMap<QString, QVariant>::iterator it = fields.begin(); it!=fields.end();++it)
		{
			if (it.key() == "seq" || it.key() == "type")
				continue;
			rec->setValue( it.key(), it.value().toString() );
		}
		//sep = rec->_sep;
		
		QByteArray data = rec->dataToSend();
		outData.append(data);
		//data.prepend( (char)('0'+seq) );
		//ui->tRecordContent->append( data );
		delete rec;
		rec=0;
		//seq++;
		//if ( seq>7 )
		//	seq=0;

		if ( cur->hasChildren() )
		{
			hOrder.append(QPair<QStandardItem *, int>(cur,curr) );
			curr=0;
		}
	}
	return true;
}

DataMapper::DataMapper()
{
}

DataMapper::~DataMapper()
{
}

void DataMapper::success(QString tok)
{
	emit sigSuccess(tok);
	
}

void DataMapper::failed(QString tok)
{
	emit sigFailed(tok);
}

bool DataMapper::processScript( QString scr )
{
	QStringList lines = scr.split('\n');
	PInstruction _actInstr;
	foreach(QString l, lines)
	{
		l = l.trimmed();		
		if ( l.isEmpty() )
			continue;
		while ( l.contains("{{$") )
		{
			QString varName = l.section( "{{$",1 ).section("}}",0,0);
			QString varVal ;// = KVStore::instance().get(varName).toString();			

			l = l.replace( QString("{{$%1}}").arg(varName), varVal );
		}

		QString cmd = l.section( QRegExp("\\s"), 0,0);

		if (cmd.contains(QRegExp("^\\s#")) )
			continue;

		else if (cmd == "define")
		{
			QString var = l.section( QRegExp("\\s"), 1,1);
			QString val = l.section( QRegExp("\\s"), 2);			
		}
		else if (cmd == "sqllink")
		{
			QString dbId = l.section( QRegExp("\\s"), 1,1);
			QString dbConnectionString = l.section( QRegExp("\\s"), 2);
			QSqlDatabase db;
			db = QSqlDatabase::addDatabase("QSQLITE",DMConnectionName);
			db.setDatabaseName( dbConnectionString );
			if(! db.open() ) //TODO: do on demand
			{
				C_ERROR("DB error. Can't open.")
				return false;
			}
			_dbs[dbId] = db;
		}

		else if (cmd == "ASTMCLEAR")
		{
			ASTMFactory::instance().clear();
		}

		else if (cmd == "SQL")
		{
			
			QString dbId = l.section( QRegExp("\\s"), 1,1);
			QString sql = l.section( QRegExp("\\s"), 2);

			if (_actInstr.isNull())
			{
				_actInstr = PInstruction(new SQLLoop(dbId,sql));
				_mainInstr = _actInstr;
			}
			else
			{
				_actInstr = PInstruction(new SQLLoop(dbId,sql));
				_mainInstr->_subInstructions.append( _actInstr );
			}
		}

		else if (cmd == "ASTMSEND")
		{			
			QString lisLink = l.section( QRegExp("\\s"), 1,1);
			QString lisLinkParams = l.section( QRegExp("\\s"), 2,2);
			QString dbId = l.section( QRegExp("\\s"), 3,3);
			QString sqlOnSuccess = l.section( "ON_SUCCESS:", 1 ).section( "ON_ERROR:", 0,0 );
			QString sqlOnError = l.section( "ON_ERROR:", 1 );
			_actInstr->_subInstructions.append( PInstruction(new ASTMSend(lisLink, lisLinkParams,dbId,sqlOnSuccess,sqlOnError)) );
		}

		else if (cmd == "SQLEND") //TODO allow more levels
		{			
			_actInstr = _mainInstr;
		}

		else if (cmd == "ASTM")
		{
			
			QString recType = l.section( QRegExp("\\s"), 1,1);
			QString fields = l.section( QRegExp("\\s"), 2);

			if (_actInstr.isNull())
			{
				C_ERROR("LIS-export script instruction error.")
				return false;
			}
			else
			{
				_actInstr->_subInstructions.append( PInstruction(new ASTMBuild(recType,fields)) );
			}
		}
	}

	if (_mainInstr.isNull())
	{
		C_ERROR("LIS-export script error.")
		return false;
	}

	return _mainInstr->process();
}

bool SQLLoop::process()
{
	QSqlDatabase db = DataMapper::instance()._dbs[_dbId];

	QSqlQuery q(db);
	
	//TODO optimize, dont replace only required names
	QString sql = _sql;
	for (QMap<QString, QString>::iterator it=_vars.begin(); it!=_vars.end(); ++it)
	{
		sql.replace( it.key(), it.value() );
	}
	
	if (!q.exec( sql ) )
	{
		C_ERROR("LIS-export script error (%s).",QS2CS(sql) );
		return false;
	}

	if(_names.isEmpty()) // get col names
	{
		QSqlRecord r = q.record();
		int cols = r.count();
		for (int i=0; i<cols; ++i)
		{
			_names << r.fieldName(i);
		}
	}

	while( q.next() )
	{
		for (int i=0; i < _names.size(); ++i )
		{
			QString rawData = q.value(i).toString();
			/*
			Allowed Characters: 7, 9, 11, 12, 13, 32–126, 128–254
			Disallowed Characters: 0–6, 8, 10, 14–31, 127, 255
			*/
			QString data;
			foreach( QChar b, rawData)
			{
				if ( b == '&' )
				{
					data+= "&E&";
				}
				else if ( b == '^' )
				{
					data+= "&S&";
				}
				else if ( b == '\\' )
				{
					data+= "&R&";
				}
				else if ( b == '|' )
				{
					data+= "&F&";
				}
				else if ( b == '\n' )
				{
					data+= "&0A&";
				}
				else if ( b == '\r')
				{
					data+= "&0D&";
				}
				else if ( b>=32 && b<=126 )
				{
					data += b;
				}
				else if ( b>=128 && b<=254 )
				{
					data += b;
				}
				else
				{
					//data+= QString("%1%2%1").arg(_sep._escapeSep).arg(QByteArray(b).toHex());
					unsigned short usc = b.unicode();
					unsigned char hb = (usc & 0xFF00) >> 8;
					unsigned char lb = (usc & 0x00FF);
					data += QString("&X%1%2%3%4&").arg(hb>0xF?"":"0").arg(QString::number(int(hb),16)).arg(lb>0xF?"":"0").arg(QString::number(int(lb),16));
				}
			}

			_vars[ QString("{{%1}}").arg(_names.at(i)) ] = data;//q.value(i).toString();
		}
		foreach( PInstruction instr, _subInstructions )
		{
			instr->process();
		}
	}
	return true;
}

SQLLoop::SQLLoop(QString db, QString sql):_dbId(db),_sql(sql)
{
}

void ASTMSendCallback::success()
{
	QSqlDatabase db = DataMapper::instance()._dbs[_dbId];

	QSqlQuery q(db);		
	
	if ( !q.exec( _sqlOnSuccess ) )
	{
		C_ERROR("SQL statement error: '%s'", QS2CS(_sqlOnSuccess) );
		DataMapper::instance().failed(_sqlOnSuccess);
	}
	else
	{
		DataMapper::instance().success(_sqlOnSuccess);
	}
	deleteLater();
}

void ASTMSendCallback::error(QString msg)
{

	QSqlDatabase db = DataMapper::instance()._dbs[_dbId];

	QSqlQuery q(db);		
	
	if (!q.exec( _sqlOnError ) )
	{
		C_ERROR("SQL statement error: '%s'", QS2CS(_sqlOnError) );
		DataMapper::instance().failed(_sqlOnError);
	}
	DataMapper::instance().failed(_sqlOnError);
	deleteLater();
}

void ASTMSendCallback::timeout()
{
	//TODO ERROR. DataMapper::instance().failed(_sql);
	deleteLater();
}

ASTMSendCallback::ASTMSendCallback(QString db, QString sqlOnSuccess, QString sqlOnError):
	_dbId(db)
	,_sqlOnSuccess(sqlOnSuccess)
	,_sqlOnError(sqlOnError),
	_timeout(5000)
{
	//QTimer * t = new QTimer(this);
	//connect(t, SIGNAL(timeout()), this, SLOT(timeout()) );
	//t->setSingleShot(true);
	//t->start(_timeout);//TODO _timeout can't be set
}


ASTMSend::ASTMSend(QString llink, QString llinkParams, QString db, QString sqlOnSuccess, QString sqlOnError ):
	_dbId(db)
	,_sqlOnSuccess(sqlOnSuccess)
	,_sqlOnError(sqlOnError)
	,_lisLink(llink)
	,_lisLinkParams(llinkParams)
{
}

bool ASTMSend::process()
{
	//TODO optimize, dont replace only required names
	QString sqlOnSuccess = _sqlOnSuccess;
	for (QMap<QString, QString>::iterator it=_vars.begin(); it!=_vars.end(); ++it)
	{
		sqlOnSuccess.replace( it.key(), it.value() );
	}
	QString sqlOnError = _sqlOnError;
	for (QMap<QString, QString>::iterator it=_vars.begin(); it!=_vars.end(); ++it)
	{
		sqlOnError.replace( it.key(), it.value() );
	}
	ASTMSendCallback *cb = new ASTMSendCallback(_dbId,sqlOnSuccess,sqlOnError);
	
	QString linkType = _lisLink.section("://",0,0);
	QString linkValue = _lisLink.section("://",1);

	if (linkType == "tcp")
	{
		TPLisSendjob sendJob(new LisSendjob());
		sendJob->_data = ASTMFactory::instance().sessionData();
		sendJob->_cb = cb;
		ConnectionManager::instance().addSendJob( sendJob );
	}
	else if (linkType == "file")
	{
		static int sessionCnt=0;
		QString resFname;// TODO

		QString fn = QString("%1/%2").arg(linkValue).arg(resFname);

		if ( QFileInfo(fn).exists() )
		{
			C_ERROR( "LIS configuration error. File conflict (exists) for %s", QS2CS(fn) );
			return false;
		}

		QFile f( fn );
		if (!f.open(QIODevice::WriteOnly))
		{
			C_ERROR("Can't send data to LIS via file: %s", QS2CS(fn) );
			return false;
		}

		QList<QByteArray> messages = ASTMFactory::instance().sessionData();
		foreach(QByteArray ln, messages)
		{
			f.write( Helpers::toLIS2_A2( ln ) );
		}		
		f.flush();	
		f.close();		
		QTimer::singleShot(  1000, cb, SLOT( success() )  );
	}
	else
	{
		QTimer::singleShot(  300, cb, SLOT( error() )  );
		//???return error;
	}

	ASTMFactory::instance().clear();

	return true;
}

bool ASTMBuild::process()
{
	PASTMRecord prec;
	if ( _recType == "X" )
	{
		ASTMFactory::instance().addSession();		
	}
	else if ( _recType == "Y" )
	{
		ASTMFactory::instance().addMessage();
	}
	else if ( _recType == "H" )
	{
		prec = PASTMRecord( new ASTMHeader("|\\^&") );
	}
	else if ( _recType == "P" )
	{
		prec = PASTMRecord( new ASTMPatient(1) );
	}
	else if ( _recType == "O" )
	{
		prec = PASTMRecord( new ASTMOrder(1) );
	}
	else if ( _recType == "R" )
	{
		prec = PASTMRecord( new ASTMResult(1) );
	}
	else if ( _recType == "C" )
	{
		prec = PASTMRecord( new ASTMComment(1) );
	}
	else if ( _recType == "L" )
	{
		prec = PASTMRecord( new ASTMTerminator(1) );
	}
	else if ( _recType == "Q" )
	{
		C_ERROR( "Request record not supported yet" );
		return false;
	}
	else if ( _recType == "M" )
	{
		C_ERROR( "Manufacturer record not supported yet" );
		return false;
	}
	else if ( _recType == "S" )
	{
		C_ERROR( "Scientific record not supported yet" );
		return false;
	}
	else
	{
		C_ERROR( "Invalid ASTM 1394 record type%s", QS2CS( _recType ) );
		return false;
	}

	QString fields = _fields;
	//TODO: optimize! s.a.
	for (QMap<QString, QString>::iterator it=_vars.begin(); it!=_vars.end(); ++it)
	{
		fields.replace( it.key(), it.value() );
	}
	if(!prec.isNull())
	{
		foreach(  QString f, fields.split(",",QString::SkipEmptyParts) )
		{
			QString name = f.section("=",0,0).trimmed();
			QString val = f.section("=",1).trimmed();
			prec->setValue(name, val);
		}
		return ASTMFactory::instance().setRecord( prec );
	}
	return false;
}

ASTMBuild::ASTMBuild(QString recType, QString fields):_recType(recType),_fields(fields)
{

}

