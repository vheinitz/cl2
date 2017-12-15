#ifndef _EXTENDED_COMBO_BOX__HG
#define _EXTENDED_COMBO_BOX__HG


#include <base/cldef.h>
#include <QStyledItemDelegate>
#include <QList>
#include <QComboBox>
#include <QPainter>





/*! \brief Extended combo box
*
* Able to display images 
*/
class CLIB_EXPORT ExtComboBox : public QComboBox
{
	Q_OBJECT

	friend class CBImageDelegate;
	
	class CBImageDelegate : public QStyledItemDelegate
	{
		ExtComboBox *_parent;
		public:
		CBImageDelegate(ExtComboBox *parent ) : QStyledItemDelegate(parent),_parent(parent){}

		~CBImageDelegate() { }

		void paint(QPainter *painter, const QStyleOptionViewItem &item, const QModelIndex &mi) const
		{
			QList<QPixmap> & flags = _parent->_data;
			int idx=0;
			for ( QList<QPixmap>::const_iterator it =  flags.constBegin(); it != flags.constEnd(); ++it)
			{		
				if ( idx == mi.row() )
				{
					QRect r= item.rect;
					QSize s( r.size().width()-4, r.size().height()-4 );					
					QPixmap spx =  (*it).scaled( s );
					painter->drawPixmap( item.rect.x()+2, item.rect.y()+2, spx );
				}
				idx++;
				//ui->cbLanguageChange->addItem(QIcon( it.value() ), "" );
			}
			/*QPixmap flag(":/res/lang/de_DE.png");
			painter->drawPixmap( item.rect(), flag.scaled(item.rect.size() ));
			*/
		}
		virtual QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const { return QSize(84,50); }
	};
	QList<QPixmap> _data;
public:
	ExtComboBox(QWidget *p):QComboBox(p)
	{
		
	}
	void setData( QList<QPixmap> data )
	{
		_data = data;
		for (int i=0; i< data.size(); ++i)
		{
			addItem( QIcon( _data.at(i) ), "" );
		}
		//this->addItem(sdata);
		setIconSize ( QSize(80, 48) );
	}
	void clear()
	{
		_data.clear();
		QComboBox::clear();
	}

protected:
	/*void paintEvent ( QPaintEvent *e )
	{

		int idx=currentIndex();
		if (idx < 0)
		{
			idx=0;
		}
		if ( currentIndex() <= _data.size() )
		{
			QComboBox::paintEvent(e);
			QPixmap curImg = _data.at( idx );
			QPainter painter(this);
			painter.drawPixmap( contentsRect()/2, curImg.scaled(contentsRect().size() ));			
		}
	}*/
};

#endif
