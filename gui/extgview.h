#ifndef _EXTENDED_GRAPHICS_VIEW__HG
#define _EXTENDED_GRAPHICS_VIEW__HG

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPointF>
#include <base/cldef.h>



 /*! \brief Extended grafics view
  *
  * Able to handle and report mouse events
  */

class CLIB_EXPORT ExtGraphicsView : public QGraphicsView
{
    Q_OBJECT
signals:
    void clicked( QPointF );
	void pressed( QPointF );
    void mouseOver( QPointF );

public:
    ExtGraphicsView(QWidget *p=0) :
            QGraphicsView(p)
    {
        setMouseTracking(true);
    }
protected:
    void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent ( QMouseEvent * event );
};

#endif
