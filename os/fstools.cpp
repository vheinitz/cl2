#include "fstools.h"
#include <QIODevice>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QFileInfoList>

QString FSTools::readAll( QString fn)
{
    QFile f(fn);
    if(!f.open(QIODevice::ReadOnly))
		return QString::null;

	return f.readAll();
}


QStringList FSTools::fromFile( QString fn)
{
    QStringList ret;
    QFile f(fn);
    f.open(QIODevice::ReadOnly);
    QTextStream ts(&f);
    while (!ts.atEnd())
    {
        ret.append(ts.readLine());
    }
    return ret;
}

bool FSTools::toFile( QStringList sl, QString fn)
{
    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly))
		return false;

    QTextStream ts(&f);
    foreach (QString s, sl)
    {
        ts << s <<"\n";
    }
    return true;
}

QMap<QString, QString> FSTools::mapFromFile( QString fn, QRegExp sep)
{
	QMap<QString, QString> ret;
    QFile f(fn);
    f.open(QIODevice::ReadOnly);
    QTextStream ts(&f);
    while (!ts.atEnd())
    {
		QString line = ts.readLine();
		QString k = line.section(sep,0,0).trimmed();
		if (k.isEmpty())
		{
			continue;
		}
		QString v = line.section(sep,1);
        ret[k] = v;
    }
    return ret;
}

bool FSTools::mapToFile( QMap<QString, QString> m, QString fn, QString sep)
{
	QFile f(fn);
	if (!f.open(QIODevice::WriteOnly))
		return false;

    QTextStream ts(&f);
	foreach (QString mk, m.keys())
    {
		ts << mk<<sep<<m[mk]<<"\n";
    }
    return true;
}

bool FSTools::freeSpaceCheck()
{
	QString wlOutputDrive = "C";//_wlXml.section(":",0,0);
	bool ok=0;
	wlOutputDrive+=":";
	long freeSpace = FSTools::getFreeSpaceInMB( wlOutputDrive , &ok);
 
	if ( !ok || (freeSpace < 1000) )
	{
		int but= QMessageBox::warning( 0,  "Warning", "Left space on hard drive for storing images may be insufficient",QMessageBox::Ok | QMessageBox::Abort );
		if ( but == QMessageBox::Abort )
		{
			//PD_LOG("Aborted by user on insufficient disc space warning")
			return false;
		}
	}
	return true;
}

bool FSTools::removeDir(const QString &dirName)
{
    QDir dir(dirName);	 
    if (dir.exists(dirName))
	{
		QFileInfoList fil = dir.entryInfoList(
			(
				QDir::NoDotAndDotDot | 
				QDir::Files |
				QDir::System | 
				QDir::Hidden  | 
				QDir::AllDirs
			),			 
			QDir::DirsFirst
		);

        foreach(QFileInfo fi, fil ) {
            if (fi.isDir()) {
                if ( !removeDir(fi.absoluteFilePath()) )
					return false;
            }
            else {
                if ( !QFile::remove(fi.absoluteFilePath()) )
					return false;
            }
 
        }
        return dir.rmdir(dirName);
    }
 
    return false;
}
