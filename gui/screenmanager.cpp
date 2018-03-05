
#include <QMenu>
#include "screenmanager.h"
#include "screen.h"

ScreenManager::ScreenManager()
:_curScr(0)
{

}

void ScreenManager::addScreen( QString scrName, IScreenLoader *scrLoader )
{
	_screens[scrName] = scrLoader;
}

void ScreenManager::openScreen( QString scrName )
{
	TScreens::iterator it = _screens.find(scrName);
	if ( it!=_screens.end() )
	{
		Screen *scr = it.value()->load(this);

		if ( _curScr )
		{
			_curScr->unload();
		}
		_curScr = it.value();
		
	}
}

void ScreenManager::openModalDialog( QString )
{

}
