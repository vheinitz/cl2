#ifndef FS_TOOLS__HG
#define FS_TOOLS__HG


#include <QString>
#include <QStringList>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QRegExp>

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

class CLIB_EXPORT FSTools{//TODO dummy. medge with changes from home
	public: static long getFreeSpaceInMB( QString , bool *ok=0){ if(ok)*ok=true; return 10000000;}
    public: static QString readAll( QString fn);
    public: static QStringList fromFile( QString fn);
	public: static bool toFile( QStringList sl, QString fn);
	public: static QMap<QString, QString> mapFromFile( QString fn, QRegExp sep);
	public: static QMap<QString, QString> mapFromText( QString data, QRegExp sepkey, QRegExp sepline=QRegExp("(\\r\\n|\\r|\\n)"));
	public: static bool mapToFile( QMap<QString, QString>m,QString fn, QString sep);
	public: static bool freeSpaceCheck();
	public: static bool removeDir(const QString &dirName);
};

struct IFile : public QFile { IFile( QString fn ):QFile(fn){ open(QIODevice::ReadOnly); } };
struct OFile : public QFile { OFile( QString fn ):QFile(fn){ open(QIODevice::WriteOnly); } };

class TOStream : public QTextStream
{
	OFile _f;

public:
	TOStream( QString fn ):_f(fn)
	{
		this->setDevice(&_f);
	}
};


#endif
