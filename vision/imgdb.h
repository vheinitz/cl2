/* ===========================================================================
 * Copyright 2015: Valentin Heinitz, www.heinitz-it.de
 * Image data base. Class for holding and fast retrieval of images
 * Author: Valentin Heinitz, 2015-12-12
 * License: MIT
 *
 ========================================================================== */

#ifndef IMAGE_DB_HG_
#define IMAGE_DB_HG_
#include <QImage>
#include <QString>
#include <QStringList>
#include <QMutex>
#include <QMap>

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

/*! Class for holding and fast retrieval of images

Currently images are stored in 3 sizes original, preview and thumbnails
Images are added to database by path. The images are renamed to a md5-shecksum extractet from image contents.
Images are retrieved from database by their md5-hash and required size
*/
class CLIB_EXPORT ImageDatabase
{
	QString _rootPath;
public:

	ImageDatabase( QString rootPath = QString::null);
	bool setRoot( QString rootPath);
	QStringList images();

	QString getImagePath( QString imghash );

	QString getPreviewPath( QString imghash );

	QString getThumbnailPath( QString imghash );

	QImage getImage( QString imghash );

	QImage getPreview( QString imghash );

	QImage getThumbnail( QString imghash );

	QString getImageHash( QString img  );

	//TODO: check possibility - conversion on first request
	QString addImage( QString img, bool *existed=0 );
	bool removeImage( QString hash );
};

class CLIB_EXPORT ImageCache
{
	ImageCache(){}
	QMap<QString, QImage> _cachedImages;
	QMutex _mapMx;

public:

	static ImageCache & instance()
	{
		static ImageCache inst;
		return inst ;
	}
	
	QImage getImage( QImage &img, QSize s );
	QImage getImage( QString imgFileName, QSize s );

};

#endif

//TODO
#if 0
class StoreImageThread : public QObject, public QRunnable
{
	Q_OBJECT

signals:
	void fileSaved(QString, QStringList);

private:
	bool _busy; // considered as uncritical for multithreading. 
	            // Set to true in constructor, and once to false 
	            // after run() finished
	int _beautify;
	QImage _image;
	TImageMap _imageMap;
	QString _path;
	bool _storeOverviewTiles;
	int _numOfFinished;
	

	QSize resIdToSize( int res )
	{
		switch( res )
		{
			////0-640x480 1-1024x768 2-1280x960 3-1632x1224 4-2048x1536 5-2560x1920
			case 0: return QSize(640,480);
			case 1: return QSize(1024,768);
			case 2: return QSize(1280,960);
			case 4: return QSize(2048,1536);
			case 5: return QSize(2560,1920);			
			case 3: 
			default:
				return QSize(1632,1224);
		}
	}

public:
	
	bool isBusy()
	{
		return _busy;
	}

	static QList<StoreImageThread*> _StoreImageTasks;

	StoreImageThread(QString path, QImage image, int beautify, TImageMap bestImages):_busy(true),_numOfFinished(0)
	{
		_beautify = beautify;
		_path = path;
		_image = image;
		_imageMap = bestImages;
		_storeOverviewTiles = _image.text("storeOverviewTiles").toInt();
		int ims = _imageMap.size();
	}

	int numberToProcess() const
	{
		int num=1;
		if ( _storeOverviewTiles )
			num += _imageMap.size();
		return num;
	}

	int numberOfFinished() const
	{
		return _numOfFinished;
	}

	void run();
};
#endif