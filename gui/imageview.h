#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <base/cldef.h>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
//#include "objectselector.h"


class ObjectSelectorInterface;

class CLIB_EXPORT ImageScene : public QGraphicsScene
{
  ObjectSelectorInterface *_selector;
  QGraphicsPixmapItem *_image;
public:
  ImageScene(  QObject *p );
  virtual ~ImageScene( );
  void setSelector(ObjectSelectorInterface *s){_selector=s;}
  void setImage( QPixmap img );
  void setMarkingActive( bool );

  virtual void keyReleaseEvent(QKeyEvent *event);

  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

};



//Interface for Selector

class CLIB_EXPORT ObjectSelectorInterface 
{

public:
  ObjectSelectorInterface( QGraphicsScene * scene=0 ){};
  virtual ~ObjectSelectorInterface( ){};
  virtual void reset(){};
  virtual void accept(){};
  virtual void mouseMoved( QPoint ){};
  virtual void mousePressed( QPoint ){};
  virtual void mouseReleased( QPoint ){};
  virtual bool isSelecting() const{return false;}
  virtual void setSelectionColor( QColor c ){};
  virtual QColor selectionColor(  )const{return QColor();}
  virtual void setActive( bool ){};
  virtual bool isActive(  )const{return false;}


/*signals:
  void canceled();
  void accepted(QPolygon);
  */
};


#endif // IMAGEVIEW_H
