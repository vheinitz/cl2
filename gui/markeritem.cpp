
#include <QtGui>

#include "markeritem.h"


MarkerItem::MarkerItem(int id, QMenu *contextMenu,
             QGraphicsItem *parent, QGraphicsScene *scene)
			 : QGraphicsPolygonItem(parent, scene),_resizing(false),_frameColor(Qt::blue),_label(""),_id(id)
{
    _contextMenu = contextMenu;

    QPainterPath path;

	_frame = QRectF(0,0,60,60);
	_resizer =QRectF(50, 50,10,10);

	setPolygon( QPolygonF( _frame ) );
    setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

QPixmap MarkerItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::blue, 8));
    painter.translate(125, 125);
    painter.drawPolyline(_frame);

    return pixmap;
}



void MarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->setPen(_frameColor);
	painter->drawRect(_frame);
	_resizer.setRect(_frame.width()-10, _frame.height()-10,10,10);
	painter->setPen(Qt::white);
	painter->drawRect(_resizer);
	painter->drawText(_frame.x()+3,_frame.y()+10,_label);

	
}

void MarkerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
	if (_contextMenu)
		_contextMenu->exec(event->screenPos());
}

QVariant MarkerItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{   
    return value;
}

void MarkerItem::resize( float width, float height )
{
	_frame.setWidth( width );
	_frame.setHeight( height );
	setPolygon( QPolygonF( _frame ) );
}

void MarkerItem::mouseMoveEvent ( QGraphicsSceneMouseEvent * ev )
{
	if ( _resizing )
	{		
		QSizeF oldSize = _frame.size();
		_frame.setWidth( _frame.width() + ev->pos().x() - ev->lastPos().x() );
		_frame.setHeight( _frame.height() + ev->pos().y() - ev->lastPos().y() );
		if (_frame.size().width() < _resizer.width() )
			_frame.setWidth( oldSize.width() );
		if (_frame.size().height() < _resizer.height() )
			_frame.setHeight( oldSize.height() );
		
		setPolygon( QPolygonF( _frame ) );

	}
	else
	{
		QGraphicsPolygonItem::mouseMoveEvent ( ev );
	}
	
}

void MarkerItem::mousePressEvent ( QGraphicsSceneMouseEvent * ev )
{
	if ( _resizer.contains(ev->pos()) )
	{
		_resizing=true;
	}
	QGraphicsPolygonItem::mousePressEvent ( ev );
}

void MarkerItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * ev )
{
	_resizing=false;
	/*if ( _resizer.contains(ev->pos()) )//proper softkey
	{
		if (_contextMenu)
		{
			QList<QAction*> act = _contextMenu->actions();
			foreach( QAction* a, act )
			{
				if (a->objectName() == "MI_Action_AddComment")
				{
					a->trigger();
				}
			}
		}
	}*/
	QGraphicsPolygonItem::mouseReleaseEvent ( ev );
}
