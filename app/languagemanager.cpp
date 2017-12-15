#include "languagemanager.h"
#include <log/clog.h>
#include <QStringList>
#include <QFile>
#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

bool LanguageManager::load ( QString dirName )
{
	QDir langDir( dirName );
	_langFile = QString("%1/lang.csv").arg(dirName);

	if ( !QFileInfo(_langFile).exists() )
	{
		C_ERROR("Error loading translation files from %s", QS2CS(dirName) )
		return false;
	}


	if ( _currentTranslator )
	{
		//warning not intended to be called twice except for testing reasons
		_currentTranslator->clear();

	}
	
	_currentTranslator = new TranslationManager(this);	
	if ( !_currentTranslator->readLangData( _langFile ) )
	{			
		return false;
	}
	qApp->installTranslator(_currentTranslator);

	QStringList langs = _currentTranslator->availableLanguages();

	int langidx=0;
	//Place en at 1st position to be default
	_langFlags["en"] = QPixmap(QString("%1/en.png").arg(dirName));
	_langCodeIdx[ langidx++ ] = "en";

	
	foreach( QString lf, langs)
	{
		QString langCode = lf.section(".",0,0);
		if ( langCode == "en" )
			continue;  //skip default
		if ( langCode == "tr" )
			continue;  //skip turkish - Q&D WA
		if ( langCode == "pt" )
			continue;  //skip portugal - Q&D WA
		QString flagFile = QString("%1/%2.png").arg(dirName).arg( langCode );
		if ( QFileInfo(flagFile).exists() )
		{
			_langFlags[langCode] = QPixmap(flagFile);
			_langCodeIdx[ langidx++ ] = langCode; // for switching language by index e.b. from a combo-box
		}
	}

	return true;
}

bool LanguageManager::changeLanguage ( int langIdx, bool async )
{
	QMap<int, QString>::const_iterator it = _langCodeIdx.find(langIdx);
	if ( it ==  _langCodeIdx.constEnd() )
	{
		C_ERROR( "Invalide language index %d", _langCodeIdx );
		return false;
	}
	return changeLanguage( it.value(), async );
}

QMap<int, QPixmap> LanguageManager::getAvailableFlags() const
{
	QMap<int, QPixmap> retMap;
	for( QMap<int, QString>::const_iterator it = _langCodeIdx.constBegin(), end = _langCodeIdx.constEnd(); it!=end; ++it )
	{
		retMap[it.key()] = _langFlags[ it.value() ];
	}
	return retMap;
}

int LanguageManager::getCurrentLangCodeIdx( ) const
{
	for( QMap<int, QString>::const_iterator it = _langCodeIdx.constBegin(), end = _langCodeIdx.constEnd(); it!=end; ++it )
	{
		if ( it.value() == _currentLanguage ) return it.key();
	}
	C_ERROR("Language not set")
	return -1;
}

QStringList LanguageManager::getAvailableLanguages( bool /*todo*/, LanguageListMode /*todo*/ ) const
{
	return _langCodeIdx.values();
}

bool LanguageManager::changeLanguage ( QString lang, bool async )
{	
	_setLanguage = lang;
	if (async)
	{
		QTimer::singleShot(500, this, SLOT(asyncSetLanguage()) );
	}
	else
	{
		asyncSetLanguage();
	}
		
	return true;//todo: remove ret
}

QString LanguageManager::getTranslation(QString orig)
{
	if ( !_currentTranslator )
	{
		C_ERROR( "Run tme error")
		return QString::null;
	}
	return _currentTranslator->translate("",orig.toUtf8().constData());

}

void LanguageManager::asyncSetLanguage()
{
	_currentLanguage = _setLanguage;
	if ( !_currentTranslator )
	{
		C_ERROR( "Run tme error")
		return;
	}

	if ( !_currentTranslator->setCurrentLanguage( _currentLanguage ) )
	{
		C_ERROR( "Language for \"%s\" not available", QS2CS(_currentLanguage) )
		return;
	}

	emit languageChanged();
}

void LanguageManager::subscribeLanguageChange( QObject* listener )
{
	connect(this, SIGNAL( languageChanged() ), listener, SLOT( processLanguageChange() ) );
}