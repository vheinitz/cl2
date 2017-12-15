#ifndef _SCREEN_H_G__
#define _SCREEN_H_G__

#include <base/cldef.h>
#include <QWidget>
#include <QString>
#include <QMainwindow>


/*! Interface for Screen (UI, page)
Main Idea is: transparently executing onEnter/onExit functions
*/

class Screen;

class CLIB_EXPORT IScreenLoader
{
public:
	virtual Screen * load( QWidget * )=0;
	virtual void unload( )=0;
};

class CLIB_EXPORT Screen : public QWidget
{
    Q_OBJECT

protected:
	QMainWindow *_mw;

public:
	Screen(QWidget *parent = 0):QWidget(parent){};
	virtual ~Screen(){};

	void setMainWindow(QMainWindow *mw){_mw=mw;}
	virtual void onEnterScreen()=0;
	virtual void onLeaveScreen()=0;
signals:
	void requestScreen( QString );
};

#endif // HG

