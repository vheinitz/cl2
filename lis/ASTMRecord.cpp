#include "ASTMRecord.h"
#include <QVariant>

const char ASTMRecord::_recordSep = 0x0D;
Separators  ASTMRecord::_sep;


QList<QByteArray> Helpers::toLIS01_A2( QByteArray tmpdata, int & seq )
{
	QList<QByteArray> dataToSendNorm;
	if (!tmpdata.isEmpty())	
	{
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
	return dataToSendNorm;
}

QByteArray Helpers::toLIS2_A2( QByteArray lis01_A2data)
{

	//<STX>X - 2 chars prefix
	//<ETX>XX<CR><LF> - 5 chars postfix
	if ( lis01_A2data.size() <= 2+5 )
		return QByteArray(); //tdo log error. invalid LIS01-A2 data

	QByteArray lis2 = lis01_A2data.mid( 2, lis01_A2data.size()-(2+5) );
	return  lis2;
}

ASTMRecord::ASTMRecord( int seq ):_seq(seq),_error(ErrNone)
{
	
}

ASTMRecord::~ASTMRecord(void)
{
}

QString ASTMRecord::toString(bool allFields)
{
	QString out;
	switch( _type )
	{
		case EHeader:
			out = "HEADER:";
			break;
		case EPatient:
			out = "PATIENT:";
			break;
		case EOrder:
			out = "ORDER:";
			break;
		case EResult:
			out = "RESULT:";
			break;
		case EComment:
			out = "COMMENT:";
			break;
		case ETerminator:
			out = "TERM:";
			break;
	}
	foreach ( QString n, _ent )
	{
		QString d = _vals[n];
		if ( !d.isEmpty() || allFields )
			out+="\t"+n+":"+d+"\n";
	}
	return out;
}

QMap<QString, QVariant> ASTMRecord::values()
{
	QMap<QString, QVariant> vals;
	foreach ( QString n, _ent )
	{
		if ( n == "seq" || n == "type")
			continue;

		vals.insert(n, QVariant(_vals[n]));
	}
	return vals;
}

bool ASTMRecord::constructRecord( QStringList  ent)
{
	_ent = ent;
	if ( _ent.contains("seq") )
		_vals["seq"] = QString::number(_seq);
	return true; //TODO may check for validness of entities
}

bool ASTMRecord::setValue( int idx, QString value)
{
	if ( idx<0 || idx >= _ent.size() )
		return false;
	setValue( _ent.at(idx), value );
	return true;
}

bool ASTMRecord::setValue( QString ent, QString value)
{
	if (ent=="type")
	{
		if (value.isEmpty())
			return false;

		//_type = value.at(0).toAscii();
		_vals[ent] = value;
		return true;
	}	
	else if (ent=="delimeter") //type head should be checked, but clear implicitly
	{
		setSeparators(value);
		_vals[ent] = value;
	}
	else if ( !_ent.contains(ent) )
		return false;
	else
		_vals[ent] = value;

	return true;
}

QByteArray ASTMRecord::dataToSend( )
{
	QByteArray data;
	QByteArray nedata; //not empty
	bool first=true;
	bool second=false; // for skiping on H
	foreach( QString ent, _ent )
	{		
		if (!first)
		{
			if ( second && _type == EHeader )
			{		
				second = false;
			}
			else
			{
				nedata+=_sep._fieldSep;
			}
		}
		else
		{
			first=false;
			second=true;
		}

		if ( !_vals[ent].isEmpty() )
		{
			data +=nedata;
			nedata.clear();
			data+=_vals[ent].toAscii();					
			
		}
	}

	return data;
}


ASTMHeader::ASTMHeader(QString delim)
{
	constructRecord(QStringList() 
		<<"type"<<"delimeter"<<"message_id"<<"password"<<"sender"
		<<"address"<<"reserved"<<"phone"<<"caps"<<"receiver"<<"comments"
		<<"processing_id"<<"version"<<"timestamp" );
	_vals["type"] = "H";
	_type=EHeader;
	setSeparators( delim );
	_vals["delimeter"] = delim;
}


ASTMPatient::ASTMPatient( int seq) : ASTMRecord( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"practice_id" <<"laboratory_id" <<"id" <<"name"
		<<"maiden_name" <<"birthdate" <<"sex" <<"race" <<"address" <<"reserved"
		<<"phone" <<"physician_id" <<"special_1" <<"special_2" <<"height" <<"weight"
		<<"diagnosis" <<"medication" <<"diet" <<"practice_field_1" 
		<<"practice_field_2" <<"admission_date" <<"admission_status" <<"location"
		);
	_vals["type"] = "P";
	_type=EPatient;
}

ASTMOrder::ASTMOrder( int seq) : ASTMRecord( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"spcmId" <<"instrSpcmId" <<"unvTestId" <<"priority" <<"created_at"
		<<"sampled_at" <<"collected_at" <<"volume" <<"collector" <<"action_code" <<"danger_code"
		<<"clinical_info" <<"delivered_at" <<"biomaterial" <<"physician" <<"physician_phone"
		<<"user_field_1" <<"user_field_2" <<"laboratory_field_1" <<"laboratory_field_2"
		<<"modified_at" <<"instrument_charge" <<"instrument_section" <<"report_type"
		);
	_vals["type"] = "O";
	_type=EOrder;
}

ASTMResult::ASTMResult( int seq) : ASTMRecord( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"unvTestId" <<"value" <<"units"
		<<"references" <<"abnormal_flag" <<"abnormality_nature" <<"status"
		<<"norms_changed_at" <<"operator" <<"started_at" <<"completed_at" <<"instrument"
		);
	_vals["type"] = "R";
	_type=EResult;
}

ASTMComment::ASTMComment( int seq) : ASTMRecord( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"source" <<"data" <<"ctype"
		);
	_vals["type"] = "C";
	_type=EComment;
}

ASTMTerminator::ASTMTerminator( int seq ) : ASTMRecord( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"code"
		);
	_vals["type"] = "L";
	_type=ETerminator;
}




ASTMMessage::ASTMMessage(void)
{
	_recNum=1;
	//_currentLevel=0;
	//_levelSeq<<1;
	_nextRecord = 0;
}

ASTMMessage::~ASTMMessage(void)
{
}

QByteArray ASTMMessage::dataToSend()
{
	if( _records.size() <= _nextRecord )
		return QByteArray();

	//int level = _records.at(_nextRecord)._level;
	QByteArray data = _records.at(_nextRecord)->dataToSend( );	
	data.prepend( '0'+_recNum );
	if ( ++_recNum > 7 )
		_recNum = 0;
	data.append( ASTM_CR );
	data.append( ASTM_ETX );
	unsigned char cs = Helpers::checkSum(data);
	data.append( Helpers::binToHex( cs ) );	
	data.append( ASTM_CR );
	data.append( ASTM_LF );
	data.prepend( ASTM_STX );
	

	return data;
}

bool ASTMMessage::atEnd()
{
	return( _records.size() <= _nextRecord );
}