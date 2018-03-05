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


ImageScene::ImageScene( QObject *p ) :QGraphicsScene(p),_image(0)
{
}

ImageScene::~ImageScene( )
{

}

void ImageScene::setMarkingActive( bool m )
{
	_selector->setActive( m );
}

void ImageScene::keyReleaseEvent(QKeyEvent *event)
{
  switch(  event->key() )
  {
      case Qt::Key_Escape:
      {
        if ( _selector ) _selector->reset();
        event->accept();
      }break;

    case Qt::Key_Enter:
      {
        if ( _selector ) _selector->accept();
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
  if ( _selector )_selector->mouseMoved( event->scenePos().toPoint() );
  event->accept();

}

void ImageScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if ( !_selector ) return;

  if(  event->button() & Qt::MouseButton::LeftButton && ! _selector->isSelecting() )
  {
    QPointF p = event->scenePos();
     _selector->mousePressed( event->scenePos().toPoint() );
     event->accept();
  }
}

void ImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if(  event->button() & Qt::MouseButton::LeftButton )
  {
      if ( _selector ) _selector->mouseReleased( event->scenePos().toPoint() );
      event->accept();
  }
}

void ImageScene::setImage( QPixmap img )
{	
	if (_image){
		removeItem(_image);
		delete _image;
	}
	clear();
	
	_image = addPixmap( img );
	//setAlignment(Qt::AlignTop | Qt::AlignLeft);
	//fitInView( scene()->sceneRect(), Qt::KeepAspectRatio );
	//_imgOffset = QPoint( scene()->sceneRect().width() / width() , scene()->sceneRect().height() / height() );
	//_selector->setImageOffset( _imgOffset );
}


/*
ImageView::ImageView( QWidget *p, QString fn) :QGraphicsView(p),_selector(0),_image(0)
{
  _scene = new QGraphicsScene(this);
  setScene( _scene );
  if ( !fn.isEmpty() )
	_image = _scene->addPixmap( QPixmap( fn ) );

  setFrameShape(QGraphicsView::NoFrame);
}

ImageView::~ImageView( )
{

}

void ImageView::setImage( QPixmap img )
{	
	if (_image){
		_scene->removeItem(_image);
		delete _image;
	}
	_scene->clear();
	
	_image = _scene->addPixmap( img );
	setAlignment(Qt::AlignTop|Qt::AlignLeft);
	fitInView( scene()->sceneRect(), Qt::KeepAspectRatio );
	//_imgOffset = QPoint( scene()->sceneRect().width() / width() , scene()->sceneRect().height() / height() );
	//_selector->setImageOffset( _imgOffset );
}


void ImageView::keyPressEvent(QKeyEvent *event)
{

}

void ImageView::keyReleaseEvent(QKeyEvent *event)
{
  switch(  event->key() )
  {
      case Qt::Key_Escape:
      {
        if ( _selector ) _selector->reset();
        event->accept();
      }break;

    case Qt::Key_Enter:
	case Qt::Key_Return:

      {
        if ( _selector ) _selector->accept();
        event->accept();
      }break;
  }
}


void ImageView::mouseMoveEvent(QMouseEvent *event)
{
  if ( _selector )_selector->mouseMoved( event->pos() );
  event->accept();

}

void ImageView::mousePressEvent(QMouseEvent *event)
{
  if ( !_selector ) return;

  if(  event->button() & Qt::MouseButton::LeftButton && ! _selector->isSelecting() )
  {
     _selector->mousePressed( event->pos() );
     event->accept();
  }
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
  if(  event->button() & Qt::MouseButton::LeftButton )
  {
      if ( _selector ) _selector->mouseReleased( event->pos() );
      event->accept();
  }
}
*/