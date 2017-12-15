#if 0
#include "objectselector.h"
#include "imageview.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QList>
#include <QPoint>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QPolygon>

State Idle("Idle"),
      FirstPoint("FirstPoint"),
      SecondPoint("SecondPoint"),
      AreaMarking("AreaMarking"),
      AreaAccept("AreaAccept"),

      RectIdle("RectIdle"),
      RectP1("RectP1"),
      RectAccept("RectAccept"),
      ANY("ANY", true);

enum {MouseDn, MouseUp, MouseMove, Accept, Reset};

enum SelectionMode{ SelRect, SelCircle, SelArea, SelSnake, SelNO};

SelectionMode _selMode = SelNO;

QList<QPoint> _path;
QGraphicsLineItem *_rubberLine = 0;
QGraphicsPolygonItem *_currentPolygon = 0;
bool marking = false;
bool accepted = false;

void resetPolygon()
{
  if ( _currentPolygon )
    _currentPolygon->scene()->removeItem( _currentPolygon );
  _currentPolygon = 0;

  if ( _rubberLine )
    _rubberLine->scene()->removeItem( _rubberLine );
  _rubberLine = 0;

  _path.clear();
  marking = false;
}

void ObjectSelector::updatePolygon()
{
  if ( _rubberLine )
  {
    _rubberLine->scene()->removeItem( _rubberLine );
    _rubberLine = 0;
  }

  if ( _currentPolygon )
  {
    QGraphicsScene * _scene = _currentPolygon->scene();
    _currentPolygon->scene()->removeItem( _currentPolygon );
    _currentPolygon = _scene->addPolygon(QPolygon( _path.toVector() ), QPen("blue"),QBrush(_selectionColor));
    _currentPolygon->setOpacity(.4);
  }

}

bool ObjectSelector::isSelecting() const
{
  return marking;
}

ObjectSelector::ObjectSelector( QGraphicsScene *scene  ) :_scene(scene)
{

  Idle.onEnter = ACTION{ _path.clear(); marking = false; };
  AreaMarking.onEnter = ACTION{marking = true; };
  AreaMarking.onExit =  ACTION{marking = false; };
  AreaAccept.onEnter =  ACTION{  };


  _markFsm = &_areaMarkFsm;
  _areaMarkFsm(ANY,         "  -->  ",  Idle,        "ON:", Reset, ACTION{ resetPolygon(); });
  _areaMarkFsm(Idle,        "  -->  ",  FirstPoint,  "ON:", MouseUp);
  _areaMarkFsm(FirstPoint,  "  -->  ",  SecondPoint, "ON:", MouseUp);
  _areaMarkFsm(SecondPoint, "  -->  ",  AreaMarking, "ON:", MouseMove);
  _areaMarkFsm(AreaMarking, "  -->  ",  AreaMarking, "ON:", MouseMove);
  _areaMarkFsm(AreaMarking, "  -->  ",  AreaAccept,  "ON:", MouseUp);
  _areaMarkFsm(SecondPoint, "  -->  ",  Idle,        "ON:", MouseUp);
  _areaMarkFsm(AreaAccept,  "  -->  ",  Idle,        "ON:", Accept, ACTION{_currentPolygon = 0; });
  _areaMarkFsm.setStartState(Idle);
  _areaMarkFsm.start();

  _rectMarkFsm(ANY,         "  -->  ",  RectIdle,    "ON:", Reset, ACTION{ resetPolygon(); });
  _rectMarkFsm(RectIdle,    "  -->  ",  RectP1,      "ON:", MouseUp );
  _rectMarkFsm(RectP1,      "  -->  ",  RectAccept,  "ON:", MouseUp );
  _rectMarkFsm(RectAccept,  "  -->  ",  RectIdle,    "ON:", Accept  );
  _rectMarkFsm.setStartState(RectIdle);
  _rectMarkFsm.start();
}

ObjectSelector::~ObjectSelector( )
{

}

void ObjectSelector::setSelectionColor( QColor c )
{
  _selectionColor = c;
}

QColor ObjectSelector::selectionColor(  )const
{
  return _selectionColor;
}

void ObjectSelector::accept( )
{

  _path.removeFirst(); _path << _path.at(0); updatePolygon();

  if ( !isSelecting() && _currentPolygon )
  {
    emit accepted(  _currentPolygon->polygon().toPolygon()  );
  }
  _markFsm->processEvent( Accept );
}

void ObjectSelector::reset( )
{
   _markFsm->processEvent( Reset );
   if ( !isSelecting() )
   {
     emit canceled( );
   }
}

void ObjectSelector::mouseMoved(QPoint pos)
{
  _markFsm->processEvent( MouseMove );
  if ( _path.size() == 1 )
  {
    if ( _rubberLine )
    {
      _scene->removeItem(_rubberLine);
    }
    _rubberLine = _scene->addLine( _path.at(0).x(),_path.at(0).y(), pos.x(), pos.y() );
  }
  else if ( _path.size() >= 2 && marking )
  {
    QPolygon old( _path.toVector() );
    QPolygon added;added <<_path.at(0) << _path.last() <<  pos;

    if ( old.intersected( added ).isEmpty() )
    {
      _path << pos;
      if ( _currentPolygon )
      {
        _scene->removeItem( _currentPolygon );
      }
      _currentPolygon = _scene->addPolygon(QPolygon( _path.toVector() ), QPen("blue"),QBrush(_selectionColor));
      _currentPolygon->setOpacity(.4);
    }
  }
}

void ObjectSelector::mousePressed( QPoint pos )
{
   _path << pos;
   _markFsm->processEvent( MouseDn );
}

void ObjectSelector::mouseReleased( QPoint pos )
{
   _markFsm->processEvent( MouseUp );
}
#endif