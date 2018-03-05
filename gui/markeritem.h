#ifndef MarkerItem_HG
#define MarkerItem_HG

#include <base/cldef.h>
#include <QGraphicsPixmapItem>
#include <QList>


class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;



class CLIB_EXPORT MarkerItem : public QGraphicsPolygonItem
{
	int _id;
public:
    enum { Type = UserType + 15 };
	

    MarkerItem(int id, QMenu *contextMenu,
        QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

	QPolygonF polygon(void)const
	{
		return QPolygonF(_frame );
	}

	int id()const {return _id;}

	QRectF frameRect()const { return QRectF(scenePos(),_frame.size() );}
	void setFrameRect(QRectF fr) { _frame = fr; setPolygon( QPolygonF( _frame ) ); update(_frame);}

	QColor frameColor() const { return _frameColor; }
	void setFrameColor(QColor fc) { _frameColor = fc; update(_frame); }

	QString label() const { return _label; }
	void setLabel(QString l) { _label = l; update(_frame); }

	QString text() const { return _text; }
	void setText(QString t) { _text = t;  }

    QPixmap image() const;
    int type() const
        { return Type;}

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
	virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

public:
	void resize( float width, float height );

private:
    QRectF _frame;
	QColor _frameColor;
	QString _label;
	QString _text;
	QRectF _resizer;
    QMenu *_contextMenu;
	bool _resizing;
};

#endif
