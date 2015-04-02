#include <filewatcher.h>
#include <QFileInfo>
#include <QDir>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QSet>
#include <QTimer>

FileWatcher::FileWatcher(QObject *parent, bool poll) :
    QObject(parent),
	_watcher(0),
	_pollChangesTimer(0),
	_poll(poll)

{
}

FileWatcher::~FileWatcher()
{
	stop();
}

void FileWatcher::start( QString fn )
{
	QString fileName =  QFileInfo(fn).canonicalFilePath();

	
	_fileName = fileName;

	if (!_poll)
	{
		if (_watcher)
		{
			return;
		}
		_watcher = new QFileSystemWatcher(this);
		_watcher->addPaths( QStringList()<<_fileName );
			
		connect( _watcher, SIGNAL( directoryChanged ( QString ) ), this, SLOT( processDirectoryChange ( QString  ) ) );
		connect( _watcher, SIGNAL( fileChanged ( QString ) ), this, SLOT( processFileChange ( QString  ) ) );
	}
	else
	{
		if (_pollChangesTimer)
		{
			return;
		}

		_pollChangesTimer = new QTimer(this);
		_pollChangesTimer->setInterval(5000);
		connect( _pollChangesTimer, SIGNAL(timeout()), this, SLOT( pollChanges() ) );
		_pollChangesTimer->start();
	}
 
}

/*! Adde a directory, for which all new created files matching to the filter are also watched
*/
void FileWatcher::start( QString logFileRegex, QString dirName )
{
	QString cleanDirName =  QFileInfo(dirName).canonicalFilePath();
	
	if(_watchedLogDirs.contains( cleanDirName ))
		return;

	start( dirName );

	WatchedLogDir wld;
	QFileInfoList fl = QDir( cleanDirName ).entryInfoList();
	wld._lastContent = fl;
	wld._logRegExp = logFileRegex;
	_watchedLogDirs[cleanDirName] = wld;


}

void FileWatcher::processDirectoryChange (  const QString & path  )
{
	if ( _changedDirs.isEmpty() )
	{
		QTimer::singleShot(1000,this, SLOT( asyncProcessDirectoryChange() ) );
	}
	_changedDirs.enqueue(QFileInfo(path).canonicalFilePath());

}
void FileWatcher::asyncProcessDirectoryChange ( )
{
	QString cleanDirName =  _changedDirs.dequeue();
	if ( _watchedLogDirs.contains(cleanDirName) )
	{
		QFileInfoList fl = QDir( cleanDirName ).entryInfoList();
		QSet<QString> newFilesSet;
		QSet<QString> oldFilesSet;
		foreach( QFileInfo fi, QDir( cleanDirName ).entryInfoList() ) { newFilesSet.insert(fi.fileName()); }
		foreach( QFileInfo fi, _watchedLogDirs[cleanDirName]._lastContent ) { oldFilesSet.insert(fi.fileName()); }

		QSet<QString> addedFiles = newFilesSet - oldFilesSet;
		foreach( QString fn, addedFiles )
		{
			QString completeFn = cleanDirName + "/" + fn;
			_watchedLogDirs[cleanDirName]._lastContent.append( QFileInfo( completeFn  ) );
			if(!_poll)
			{
				_watcher->addPaths( QStringList()<<completeFn );
			}
			else
			{
				_watchedLogFiles[ completeFn ] = 0;
			}
		}

		QSet<QString> removedFiles = oldFilesSet - newFilesSet;
	
		if ( !removedFiles.isEmpty() )
		{
			_watchedLogDirs[cleanDirName]._lastContent.clear();
			foreach( QString fn, newFilesSet )
			{
				QString completeFn = cleanDirName + "/" + fn;
				_watchedLogDirs[cleanDirName]._lastContent.append( QFileInfo( completeFn  ) );
			}
			
			foreach( QString fn, removedFiles )
			{
				QString completeFn = cleanDirName + "/" + fn;
				if(!_poll)
				{
					_watcher->removePath( completeFn );
				}
				else
				{
					_watchedLogFiles.remove( completeFn );
				}
			}
		}
	}
}

void FileWatcher::processFileChange (  const QString & path  )
{
	QFileInfo fi(path);
	QString cleanFilePath =  fi.canonicalFilePath();
	QByteArray data;
	int lastPosition = _watchedLogFiles[ cleanFilePath ];
	if (fi.size() != lastPosition)
	{
		QFile f(cleanFilePath);
		f.open(QIODevice::ReadOnly);
		f.seek( lastPosition );
		data = f.readAll();
		_watchedLogFiles[ cleanFilePath ] = f.pos();
		emit fileContentChanged( path, data );
	}
}

void FileWatcher::pollChanges()
{
	foreach( QString d, _watchedLogDirs.keys() )
	{
		processDirectoryChange (  d  );
	}

	foreach( QString d, _watchedLogFiles.keys() )
	{
		processFileChange (  d  );
	}

}

void FileWatcher::stop()
{
	
	
	if (!_watcher)
	{
		return;
	}

	_watcher->deleteLater();
	_watcher=0;

}


