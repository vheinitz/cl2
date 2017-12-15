#ifndef ASMT_RECORD_H
#define ASMT_RECORD_H
#include <QMap>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QDateTime>
#include <QSharedPointer>
#include "cldef.h"


//Record start
static const char ASTM_STX = 0x02;
//Record end
static const char ASTM_ETX = 0x03;
//Record chunk end
static const char ASTM_ETB = 0x17;
//Session end
static const char ASTM_EOT = 0x04;
//Session start
static const char ASTM_ENQ = 0x05;
//Record or session accepted
static const char ASTM_ACK = 0x06;
//Record or session rejected
static const char ASTM_NAK = 0x15;

//Line termination
static const char ASTM_LF  = 0x0A;
static const char ASTM_CR  = 0x0D;

class CLIB_EXPORT Helpers
{
public:
	static QString binToHex( unsigned char b ) 
	{
		QString hex;
		static const char bin2Hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
		hex += bin2Hex[  b >> 4 ];
		hex += bin2Hex[  b & 0xF ];
		return hex;
	}
	static QString timestamp()
	{
		return QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	}

	static QString dataToString(QByteArray ba)
	{		
		QString d;
		for (int i=0; i<ba.size();++i)
		{
			switch (ba.at(i))
			{
				case ASTM_LF:
					d += "<LF>"; break;
				case ASTM_CR:
					d += "<CR>"; break;
				case ASTM_EOT:
					d += "<EOT>"; break;
				case ASTM_STX:
					d += "<STX>"; break;
				case ASTM_ETX:
					d += "<ETX>"; break;
				case ASTM_ETB:
					d += "<ETB>"; break;
				case ASTM_ENQ:
					d += "<ENQ>"; break;
				case ASTM_ACK:
					d += "<ACK>"; break;
				case ASTM_NAK:
					d += "<NAK>"; break;
				default:
				{
					if ( static_cast<unsigned char>(ba.at(i)) < 0x20)
					{
						d +="[";
						unsigned char b = static_cast<unsigned char>(ba.at(i));
						d += Helpers::binToHex(  b );
						d +="]";
					}
					else
						d+=ba.at(i);
				}
			}
		}
		return d;
	}

	static unsigned char checkSum( QByteArray data )
	{
		unsigned int cs=0;
		for(int i=0; i < data.size();++i)
		{
			cs+=static_cast<unsigned char>(data.at(i));
		}
		return (cs %256 ) & 0xFF;
	}

	static QList<QByteArray> toLIS01_A2( QByteArray lis2_A2data, int & seq );
	static QByteArray toLIS2_A2( QByteArray lis01_A2data);

};

typedef QMap<QString, QString> TRecordValues;

enum RecordType{
        ESession='X',
        EMessage='Y',
        EHeader='H',
        ERequest='Q',
        EPatient='P',
        EOrder='O',
        EResult='R',
        EComment='C',
        EManufacturer='M',
        EScientific='S',
        ETerminator='L'
};



struct CLIB_EXPORT Separators
{
	char _fieldSep;
	char _repeatSep;
	char _componentSep;
	char _escapeSep;
	//RecordType _type;
	Separators():
		_fieldSep('|'),
		_repeatSep('\\'),
		_componentSep('^'),
		_escapeSep('&')
	{}
};

enum ARError{ErrNone, ErrUnexpectedRecord, ErrInvalidDelimiters};

class CLIB_EXPORT ASTMRecord : public QObject
{
protected:
	QStringList _ent;
	TRecordValues _vals;
	int _seq;
	
public:
	static Separators _sep;
	void setSeparators( QString delim )
	{
		if (delim.size()<4)
			return;
		_sep._fieldSep = delim.at(0).toAscii();
		_sep._repeatSep = delim.at(1).toAscii();
		_sep._componentSep = delim.at(2).toAscii();
		_sep._escapeSep = delim.at(3).toAscii();
	}
	static const char _recordSep;
	RecordType _type;
	ARError _error;
	

	bool constructRecord( QStringList  ent);
	bool setValue( QString ent, QString value);
	bool setValue( int, QString value);
	bool isValid();
	QString toString(bool allFields=false);
	QByteArray dataToSend( );
	QMap<QString, QVariant> values();

	QStringList fields() {
		return _ent;
	}
	
	ASTMRecord( int seq=-1 );
	virtual ~ASTMRecord(void);
};



class CLIB_EXPORT ASTMHeader: public ASTMRecord
{
	public:
		ASTMHeader(QString delim);
};

class CLIB_EXPORT ASTMPatient: public ASTMRecord
{
	public:
		ASTMPatient( int seq);
};

class CLIB_EXPORT ASTMOrder: public ASTMRecord
{
	public:
		ASTMOrder( int seq);
};

class CLIB_EXPORT ASTMResult: public ASTMRecord
{
	public:
		ASTMResult( int seq);
};

class CLIB_EXPORT ASTMComment: public ASTMRecord
{
	public:
		ASTMComment( int seq);
};

class CLIB_EXPORT ASTMTerminator: public ASTMRecord
{
	public:
		ASTMTerminator(int);
};

typedef QSharedPointer<ASTMRecord> PASTMRecord;

class CLIB_EXPORT ASTMParser
{
	public:
		ASTMParser(QString delim);
		PASTMRecord parse( QString data );


};


class CLIB_EXPORT ASTMMessage : public QObject
{
	int _recNum;
	int _nextRecord;
	
public:
	QList <PASTMRecord> _records;
	ASTMMessage(void);
	virtual ~ASTMMessage(void);
	QByteArray dataToSend();
	bool atEnd();

	void ack()
	{
		++_nextRecord;
	}

	void nack()
	{
		_nextRecord=0;
	}
};

#endif
