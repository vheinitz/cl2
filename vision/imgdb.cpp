/* ===========================================================================
 * Copyright 2015: Valentin Heinitz, www.heinitz-it.de
 * Image data base. Class for holding and fast retrieval of images
 * Author: Valentin Heinitz, 2015-12-12
 * License: MIT
 *
 ========================================================================== */

#include "imgdb.h"
#include <QRgb>
#include <QPainter>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>


ImageDatabase::ImageDatabase( QString rootPath)
{
	setRoot( rootPath);
}

bool ImageDatabase::setRoot( QString rootPath)
{
	if( !QFileInfo(rootPath).exists() )
		return false;

	_rootPath = rootPath;
	QDir().mkpath( _rootPath );
	QDir().mkpath( _rootPath+"/img" );
	QDir().mkpath( _rootPath+"/preview" );
	QDir().mkpath( _rootPath+"/tn" );
	return true;
}

QStringList ImageDatabase::images()
{
	QStringList tmp =  QDir(_rootPath + "/img" ).entryList( QStringList()<<"*.png" );
	return tmp;
}

QString ImageDatabase::getImagePath( QString imghash )
{
	QString imgpath = QString("%1/img/%2.png").arg( _rootPath, imghash );
	if( !QFileInfo(imgpath).exists() )
		return "";
	return imgpath;
}

QString ImageDatabase::getPreviewPath( QString imghash )
{
	QString imgpath = QString("%1/preview/%2_pv.jpg").arg( _rootPath, imghash );
	if( !QFileInfo(imgpath).exists() )
		return "";
	return imgpath;
}

QString ImageDatabase::getThumbnailPath( QString imghash )
{
	QString imgpath = QString("%1/tn/%2_tn.jpg").arg( _rootPath, imghash );
	if( !QFileInfo(imgpath).exists() )
		return "";
	return imgpath;
}


QImage ImageDatabase::getImage( QString imghash )
{
	return QImage(getImagePath(imghash));
}

QImage ImageDatabase::getPreview( QString imghash )
{
	return QImage(getPreviewPath(imghash));
}

QImage ImageDatabase::getThumbnail( QString imghash )
{
	return QImage(getThumbnailPath(imghash));
}

QString ImageDatabase::getImageHash( QString img  )
{		
	QFile isf( img );
	isf.open(QIODevice::ReadOnly);				
	return QString(QCryptographicHash::hash( isf.readAll() ,QCryptographicHash::Md5).toHex());
}

bool ImageDatabase::removeImage( QString imghash )
{		
	QString imgDstn = QString("%1/img/%2.png").arg( _rootPath, imghash );
	bool ok=true;
	ok &= QFileInfo(imgDstn).exists();
	ok &= QFile::remove( imgDstn );
	imgDstn = QString("%1/preview/%2_pv.jpg").arg( _rootPath, imghash );
	ok &= QFile::remove( imgDstn );
	imgDstn = QString("%1/tn/%2_tn.jpg").arg( _rootPath, imghash );
	ok &= QFile::remove( imgDstn );
	return ok;
}


//TODO: check possibility - conversion on first request
QString ImageDatabase::addImage( QString img, bool *existed )
{		
	QFile isf( img );
	isf.open(QIODevice::ReadOnly);				
	QString imghash = QString(QCryptographicHash::hash( isf.readAll() ,QCryptographicHash::Md5).toHex());
	isf.close();	

	QString imgDstn = QString("%1/img/%2.png").arg( _rootPath, imghash );

	if ( QFileInfo(imgDstn).exists()  )
	{
		if (existed)
			*existed = true;		
	}
	else
	{
		if (existed)
			*existed = false;

		if ( !QFile::copy(img,imgDstn) )
		{
			return ""; // is error						
		}

		QImage tmpimg(img);
		QImage preview = tmpimg.scaled( 800,600 );
		imgDstn = QString("%1/preview/%2_pv.jpg").arg( _rootPath, imghash );
		preview.save( imgDstn );

		QImage tn = preview.scaled( 160,120 );
		imgDstn = QString("%1/tn/%2_tn.jpg").arg( _rootPath, imghash );
		tn.save( imgDstn );
	}

	return imghash;
}

	
QImage ImageCache::getImage( QImage &img, QSize s )
{
	QMutexLocker dbml(&_mapMx);
	//test without return img;
	QString hash = img.text( "hashkey" );
	if ( img.width() < 2560 )
	{
		return img.scaled(s);
	}
	else if (hash.isEmpty())
	{
		hash = QCryptographicHash::hash( QByteArray( (const char *)img.bits(), img.byteCount()), QCryptographicHash::Md5).toBase64();
		img.setText( "hashkey", hash );
		QImage small = img.scaled(s);
		QString hash_s = QString( "%1_%2_%3" ).arg(hash).arg(s.width()).arg(s.height());
		small.setText( "hashkey", hash_s );
		_cachedImages.insert( hash_s, small );
		return small;
	}
	else
	{
		QString key = QString( "%1_%2_%3" ).arg(hash).arg(s.width()).arg(s.height());
		if ( _cachedImages.contains(key) )
			return _cachedImages[key];
		
		QImage small = img.scaled(s);
		_cachedImages.insert( QString( "%1_%2_%3" ).arg(hash).arg(s.width()).arg(s.height()), small );
		return small;
	}
}

//TODO
QImage ImageCache::getImage( QString imgFileName, QSize  )
{
	return QImage();
}

///////////////// TODO
#if 0

void StoreImageThread::run()
{
	_numOfFinished=0;
	bool saved=false;
	QFile img(_path);
	img.remove();

//* TODO: Stitching is not thread-safe!!! Fix!!!
	int ims = _imageMap.size();
	struct ImgRect
	 {
		QImage *_img;
		QRect _rect;
		int _afval;
		int _zPos;
		ImgRect():_img(0),_afval(0),_zPos(0){}
	 };	 

	int _tilesXY = 20;

	ims = _imageMap.size();
	int bestZPos = _image.text("zPosition").toInt();
			

	if ( !_storeOverviewTiles && !_imageMap.isEmpty() ) //Not overview image
	{
		int tw = _image.width() / _tilesXY;
		int th = _image.height() / _tilesXY;
		typedef QMap<int, ImgRect> ImgRectMatr;
		ImgRectMatr irm;
		
		for (TImageMap::Iterator it = _imageMap.begin(); it!=_imageMap.end(); ++it)
		{
			int curZPos = it.key();
//test				it->save(QString("c:/aeskuimg/st/st_%1.png").arg(it.key()));
			int i=0;
			for( int y=0; y<_image.height(); y+=th )
			{
				for( int x=0; x<_image.width(); x+=tw )			
				{
					QRect r(x,y,tw,th);
					QImage tmp = it.value().copy(r);
					//2.0.5 int fval = PDImageProcessing::calculateFocusValue(tmp, 1, QRect(QPoint(0,0),r.size()) );
					int fval = PDImageProcessing::calculateFocusValue(tmp, 6, QRect(QPoint(0,0),r.size()) );
					if (fval >= irm[i]._afval)
					{
						 irm[i]._img = &(it.value());
						 irm[i]._afval = fval;
						 irm[i]._rect = r;	
						 irm[i]._zPos = curZPos;	
					}
					++i;
				}
			}
		}

		//QSize is = QSize( _image.size().width(), _image.size().height() );
		//QImage outimg( is, QImage::Format_RGB32);
		QImage outimg(_image);
		QPainter p(&outimg);
		p.setPen( Qt::red );
	//	p.setCompositionMode(QPainter::CompositionMode_Source);
	//	p.fillRect(outimg.rect(), Qt::blue);
		int s = irm.size();
		int ii=0;
		QString stitchingData;
		for (ImgRectMatr::iterator it = irm.begin(); it != irm.end(); ++it)
		{
			QRect r = it.value()._rect;
			int afval = it.value()._afval;
			QImage tmp;
			/*TODO check if there any effect at all: whenn difference in Z too big (>2u) => no stitching
			int zPos = it.value()._zPos;
			int zPosDiff = qAbs(zPos - bestZPos );
			if ( zPosDiff > 1600 )
				tmp = _image.copy(r);
			else*/
				tmp = it.value()._img->copy(r);
//test				tmp.save("c:/aeskuimg/st/tmp.png");
			p.setCompositionMode(QPainter::CompositionMode_Source);
			p.drawImage( r, tmp );
//test				p.drawRect( r );
			stitchingData += QString("%1 %2;").arg(ii).arg(it.value()._zPos);
			++ii;
		}
		p.end();
//test			outimg.save("c:/aeskuimg/st/st.png");
		_image = outimg;
		_image.setText( "tilesStat", stitchingData );
		_imageMap.clear();
	}
	if ( _beautify!=0 )
	{
		//Save exif before passing image to openCV
		QStringList tk = _image.textKeys();
		QMap<QString, QString> texts;
		foreach( QString k, tk )
		{
			texts[k] = _image.text(k);
		}

		//TODO make configurable: saving original image
		//saved = _image.save(_path+"_orig.png");
		if ( _beautify & 1 )
			PDImageProcessing::reduceBgNoise( _image );
		if ( _beautify & 2 )
			PDImageProcessing::sharpen(_image);

		//Restore exif after passing image to openCV
		foreach( QString k, texts.keys() )
			_image.setText( k,texts[k] );
	}

	

	saved = _image.save(_path);
	emit fileSaved( _path, QStringList() << _image.text("slide_type") );
	_numOfFinished++;

	//Q&D store all images of overview. TODO: hand over as parameter
	
	if ( _storeOverviewTiles )
	{
		int overviewTilesHqTh = _image.text("overviewTilesHqTh").toInt();
		int overviewTilesHqRes = _image.text("overviewTilesHqRes").toInt();
		int overviewTilesLqRes = _image.text("overviewTilesLqRes").toInt();
		
		QString tilespath = _path+".tiles";
		QDir().mkpath( tilespath );

		
		QFile tilesInfo( tilespath+"/size.info" );
		tilesInfo.open(QIODevice::WriteOnly);
		tilesInfo.write( QString("%1").arg(_imageMap.size()).toAscii() );
		tilesInfo.close();

		foreach( int i, _imageMap.keys() )
		{
			PDImageInfo iminf;
			PDImageProcessing::getImageInfo(_imageMap[i], iminf);
			QSize osize = resIdToSize( overviewTilesHqRes );
			QString imgtype="png";
			if ( iminf._histogramWidth < overviewTilesHqTh )
			{
				imgtype="jpg";
				osize = resIdToSize( overviewTilesLqRes );
			}

			//TODO: check if scaling needed at all
			_imageMap[i].scaled(osize).save( QString("%1/%2%3.%4" ).arg(tilespath).arg(i>=10?"0":"00").arg(i).arg(imgtype) );
			_numOfFinished++;
		}
	}
    if (!saved)
	{
        //TODO: emit errors from thread. Logger not thread-safe!!! 
		//C_ERROR("Can't save image in file: %s", QS2CS( _path ))
    }
	_busy=false;
	_StoreImageTasks.removeAll( this );
}

QList<StoreImageThread*>  StoreImageThread::_StoreImageTasks;


#endif
