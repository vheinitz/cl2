#ifndef EXTIMAGEVIEWER_H
#define EXTIMAGEVIEWER_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImage>
#include <base/cldef.h>



class CLIB_EXPORT ExtImageViewer : public QWidget
{
    Q_OBJECT
public:
    ExtImageViewer(QWidget *parent = 0);
    void setImage( QImage );

signals:
    
public slots:
    void loadImage( QString );
    
private:
    QImage _currentImage;
    QGraphicsScene _imageScene;
    QGraphicsView *_view;
    double _currentScaleFactor;
};

#endif // EXTIMAGEVIEWER_H
