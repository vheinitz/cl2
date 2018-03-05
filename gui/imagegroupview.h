#ifndef ImageGroupView_H
#define ImageGroupView_H

#include <base/cldef.h>
#include <QFuture>
#include <QFutureWatcher>
#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class ImageGroupView;
}

class ImageGroupView : public QWidget
{
    Q_OBJECT
    
public:
    explicit ImageGroupView(QWidget *parent = 0);
    ~ImageGroupView();
	void setImages( QStringList );
	void setId(int id);
	QString id()const
	{
		return QString("%1%2").arg(_id>9?"":"0").arg(_id);
	}	
	int loadImages( QStringList );
	void setChecked( bool );
	bool isChecked();
    
private slots:
    void on_cbIsNegative_clicked();
	void processImagesReady();

    void on_bScrollRight_clicked();

    void on_bScrollLeft_clicked();

    void on_lvImages_activated(const QModelIndex &index);

    void on_lvImages_clicked(const QModelIndex &index);

    void on_lvImages_doubleClicked(const QModelIndex &index);

private:
    Ui::ImageGroupView *ui;
    QList<QImage> _imageList;
	QFuture<int> _imageLoader;	
	QFutureWatcher<int> _watcher;
    QStandardItemModel _images;
	int _id;
};

#endif // ImageGroupView_H
