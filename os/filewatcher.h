#ifndef _FileWatcher_HG
#define _FileWatcher_HG


#include <QObject>
#include <QByteArray>
#include <QString>
#include <QQueue>
#include <QDir>
#include <QTimer>
#include <QFileInfo>
#include <QFileInfoList>
#include <QMap>

class QFileSystemWatcher;

struct WatchedLogDir
{
	QFileInfoList _lastContent;
	QString _logRegExp;
};

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

class CLIB_EXPORT FileWatcher : public QObject
{
    Q_OBJECT
private:
	QFileSystemWatcher *_watcher;
	QTimer * _pollChangesTimer;
	QString _fileName;
	QQueue<QString> _changedFiles;
	QQueue<QString> _changedDirs;
	QMap<QString, QFileInfoList> _watchedDirs;
	QMap<QString, WatchedLogDir> _watchedLogDirs;
	QMap<QString, int > _watchedLogFiles;
	bool _poll;

private slots:
	void processDirectoryChange( const QString & path );
	void asyncProcessDirectoryChange(  );
	void processFileChange( const QString & path );
	void pollChanges();

public:
    FileWatcher(QObject *parent = 0, bool poll=false);
	virtual ~FileWatcher();

signals:
	void changed();
	void fileContentChanged( QString path, QByteArray newData );
    
public slots:
	void start( QString fileName );
	void start( QString logFileRegex, QString dirName );
    void stop();  
};

#endif // LIS_PROXY_HG_
