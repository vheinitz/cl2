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

class CLIB_EXPORT ObjectSelectorInterface : public QObject
{
Q_OBJECT

public:
  ObjectSelectorInterface( QGraphicsScene * scene=0 ){};
  virtual ~ObjectSelectorInterface( ){};
  virtual void reset()=0;
  virtual void accept()=0;
  virtual void mouseMoved( QPoint )=0;
  virtual void mousePressed( QPoint )=0;
  virtual void mouseReleased( QPoint )=0;
  virtual bool isSelecting() const=0;
  virtual void setSelectionColor( QColor c )=0;
  virtual QColor selectionColor(  )const=0;
  virtual void setActive( bool )=0;
  virtual bool isActive(  )const=0;


signals:
  void canceled();
  void accepted(QPolygon);
};


#endif // IMAGEVIEW_H
