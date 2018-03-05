#include "extimageviewer.h"
#include <QBoxLayout>
#include <QPixmap>

ExtImageViewer::ExtImageViewer(QWidget *parent) :
    QWidget(parent)
{
	_view = new QGraphicsView;	
	_view->setScene( &_imageScene );
	setLayout( new QVBoxLayout );
	layout()->addWidget(_view);
	this->setMinimumHeight(480);
	
}

void ExtImageViewer::loadImage( QString imgfn )
{
	setImage(QImage(imgfn));
}

void ExtImageViewer::setImage( QImage img )
{
	_currentImage = img;
	_imageScene.addPixmap(  QPixmap::fromImage(_currentImage) );
}