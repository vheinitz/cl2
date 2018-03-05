#include "ImageGroupView.h"
#include "ui_ImageGroupView.h"
#include <QtCore>

enum DataRole{ IconRole=Qt::UserRole+1, FileNameRole=Qt::UserRole+2 };

ImageGroupView::ImageGroupView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageGroupView),
	_id(0)
{
    ui->setupUi(this);
	ui->lvImages->setModel( &_images );
	
}

ImageGroupView::~ImageGroupView()
{
	_watcher.cancel();
	_watcher.waitForFinished();
    delete ui;
}

void ImageGroupView::on_cbIsNegative_clicked()
{

}

int ImageGroupView::loadImages( QStringList il )
{
	_images.clear();
	foreach ( QString img, il )
	{
        //_images.insertRow(_images.rowCount());        
        _imageList.append( QImage( img ).scaled(640,480) );
	}
	return 0;
}

void ImageGroupView::setImages( QStringList il )
{
	_imageLoader = QtConcurrent::run(this, &ImageGroupView::loadImages, il );	
	connect(&_watcher, SIGNAL(finished()), this, SLOT(processImagesReady()));
	_watcher.setFuture( _imageLoader );
}

void ImageGroupView::setId(int id)
{
	_id=id;
    ui->lCaption->setText(QString("Well %1").arg(_id));
}

void ImageGroupView::setChecked( bool chk )
{
	ui->cbIsNegative->setChecked( chk );
}

bool ImageGroupView::isChecked()
{
	return ui->cbIsNegative->isChecked();
}

void ImageGroupView::processImagesReady()
{
	if( !_imageList.isEmpty() )
	{
		ui->lCurrentImage->setPixmap( QPixmap::fromImage( _imageList.at(0).scaled(ui->lCurrentImage->contentsRect().size() ) ) );
		//ui->lCurrentImage->setText("Hello")	;
	}
	foreach ( QImage img, _imageList )
	{
        _images.insertRow(_images.rowCount());
        _images.setItem(
			_images.rowCount()-1,
			new QStandardItem(
				QIcon(
					QPixmap::fromImage(img).scaled(70,50)					
				)
				,""
			)
        );
	}
	show();
}
void ImageGroupView::on_bScrollRight_clicked()
{

}

void ImageGroupView::on_bScrollLeft_clicked()
{

}

void ImageGroupView::on_lvImages_activated(const QModelIndex &index)
{
	if( _imageList.size() > index.row() )
	{
		ui->lCurrentImage->setPixmap( QPixmap::fromImage( _imageList.at(index.row()).scaled(ui->lCurrentImage->contentsRect().size() ) ) );
		//ui->lCurrentImage->setText("Hello")	;
	}
}

void ImageGroupView::on_lvImages_clicked(const QModelIndex &index)
{
    on_lvImages_activated(index);
}

void ImageGroupView::on_lvImages_doubleClicked(const QModelIndex &index)
{

}
