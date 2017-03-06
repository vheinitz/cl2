#ifndef OBJECTSELECTOR_H
#define OBJECTSELECTOR_H

#if 0
#include <QWidget>
#include <QGraphicsScene>
#include <QPoint>
#include "fsm.h"

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

class CLIB_EXPORT ObjectSelector : public QObject
{
  Q_OBJECT
  QGraphicsScene * _scene;
  FSM * _markFsm;
  FSM  _areaMarkFsm;
  FSM  _circleMarkFsm;
  FSM  _rectMarkFsm;
  FSM  _snakeMarkFsm;
  QColor _selectionColor;

public:
  ObjectSelector( QGraphicsScene * scene );
  virtual ~ObjectSelector( );
  void reset();
  void accept();
  void mouseMoved( QPoint );
  void mousePressed( QPoint );
  void mouseReleased( QPoint );
  bool isSelecting() const;
  void setSelectionColor( QColor c );
  QColor selectionColor(  )const;

private:
  void updatePolygon();
signals:
  void canceled();
  void accepted(QRegion);
};

#endif
#endif // OBJECTSELECTOR_H
