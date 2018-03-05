#include "extgview.h"


void ExtGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (scene())
    {
		QRectF sr = this->sceneRect();
		QPointF cp= event->pos();
		QPointF cp1 = this->mapFromScene(event->pos());
		QPointF cp2 = this->mapToScene(event->pos());

        emit pressed( mapToScene( event->pos()) );
    }
	QGraphicsView::mousePressEvent(event);
}

void ExtGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (scene())
    {
		QRectF sr = this->sceneRect();
		QPointF cp= event->pos();
		QPointF cp1 = this->mapFromScene(event->pos());
		QPointF cp2 = this->mapToScene(event->pos());

        emit clicked( mapToScene( event->pos()) );
    }
	QGraphicsView::mouseReleaseEvent(event);
}

void ExtGraphicsView::mouseMoveEvent(QMouseEvent *event)
{

    if (scene())
    {
        emit mouseOver( mapToScene( event->pos()) );
    }
	QGraphicsView::mouseMoveEvent(event);
}
