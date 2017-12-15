#include "imagelistmodel.h"
#include <QPainter>
#include <QAbstractItemView>

void ImageListModel::clear()
{
	_data.clear();
    _viewImage.clear();
    reset();
}

int ImageListModel::rowCount(const QModelIndex &) const
{
	return _data.size();
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const
{
	QVariant val;
	if ( role == Qt::DisplayRole )
	{
		int idx = index.row();
		if ( idx < _data.size() )
		{
			val.setValue( _data.at(idx) );
		}
	}
	return val;
}

void ImageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, QColor::fromRgb(150, 175, 50));

	ImageListModel *viewModel = qobject_cast<ImageListModel *>(_view->model());
    if( viewModel )
    {
        if( viewModel->_viewImage.size() > index.row()
            && viewModel->_viewImage.at(index.row()).isNull() )
        {
	        QImage img = viewModel->data(index).value<QImage>();			
            viewModel->_viewImage[index.row()] = QImage( option.rect.size(),QImage::Format_ARGB32 );
            QPainter imagePainter(&(viewModel->_viewImage[(index.row())]));
			imagePainter.fillRect(0,0, img.width(), img.height(), QColor::fromRgb(150, 150, 150));
			imagePainter.setFont(QFont("Verdana", 12));
			imagePainter.drawText( 10,15, QString("Image %1").arg(index.row()+1));
			imagePainter.drawImage( 10, 30, img.scaled( QSize(240, 160), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );

			QString clsf = img.text("classification");
			QRect classRect( 200, 120, 40, 40);
		    imagePainter.fillRect(classRect, QColor(220, 220, 220, 180));
		    imagePainter.setPen(QColor(0, 0, 0));		
		    imagePainter.setFont(QFont("Verdana", 16,2));
			
		    imagePainter.drawText( classRect, Qt::AlignHCenter | Qt::AlignVCenter, clsf  );
			imagePainter.end();
        }
        painter->drawImage( option.rect, viewModel->_viewImage.at(index.row()) );
    }    
}

QSize ImageDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
	return QSize(250, 200);
}