#include "imageview.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QList>
#include <QPoint>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QPolygon>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QPolygon>
#include <QGraphicsSceneMouseEvent>


ImageScene::ImageScene( QObject *p ) :QGraphicsScene(p),_image(0)//,_selector(0)
{
}

ImageScene::~ImageScene( )
{

}

void ImageScene::setMarkingActive( bool m )
{
	//_selector->setActive( m );
}

void ImageScene::keyReleaseEvent(QKeyEvent *event)
{
  switch(  event->key() )
  {
      case Qt::Key_Escape:
      {
        //if ( _selector ) _selector->reset();
        event->accept();
      }break;

    case Qt::Key_Enter:
      {
        //if ( _selector ) _selector->accept();
        event->accept();
      }break;
  }
}

void ImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QPointF p = event->scenePos();
  if ( !sceneRect().contains( event->scenePos() ) )
  {
    event->accept();
    return;
  }
  //if ( _selector )_selector->mouseMoved( event->scenePos().toPoint() );
  event->accept();

}

void ImageScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  //if ( !_selector ) return;

  if(  event->button() & Qt::MouseButton::LeftButton  )
  {
    QPointF p = event->scenePos();
    // if ( _selector ) _selector->mousePressed( event->scenePos().toPoint() );
     event->accept();
  }
}

void ImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if(  event->button() & Qt::MouseButton::LeftButton )
  {
      //if ( _selector ) _selector->mouseReleased( event->scenePos().toPoint() );
	  emit clicked( event->scenePos().toPoint() );
      event->accept();
  }
}

void ImageScene::clear(  )
{
	_image=0;
	QGraphicsScene::clear();
}

void ImageScene::setImage( QPixmap img )
{	
	if (_image){
		removeItem(_image);
		delete _image;
	}
	clear();
	this->setSceneRect( QRect(QPoint(0,0),img.size()) );
	
	_image = addPixmap( img );
}
