#ifndef _LANGUAGE_MANAGER_HG_
#define _LANGUAGE_MANAGER_HG_

#include <cldef.h>
#include <QString>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QVariantList>
#include "translationmanager.h"


class AsyncMessage: public QObject
{
Q_OBJECT

	QObject *_r; //<receiver
	QVariantList _args;
public:
	AsyncMessage( QObject * r, const char * slot, QVariantList args );

};

/*! @brief Class for switching between available application languages
*/
class CLIB_EXPORT LanguageManager : public QObject
{
	Q_OBJECT

signals:
	void languageChanged();
private:
	LanguageManager():_currentTranslator(0){};
	QString _currentLanguage;
	QString _setLanguage;

private slots:
	void asyncSetLanguage();

public:
	enum LanguageListMode
	{
		ShowInEnglish,
		ShowInCurrentLanguage,
		ShowInNativeLanguage
	};
	static LanguageManager &instance()
	{
		static LanguageManager inst;
		return inst;		
	}
    QString currentLanguage() const { return _currentLanguage.section("_",0,0);}
	void subscribeLanguageChange( QObject* );	
	bool load ( QString dir );
	bool changeLanguage ( int langIdx, bool async=false );
	bool changeLanguage ( QString langCode, bool async=false );

	QString getTranslation(QString orig);

	QMap<int, QPixmap> getAvailableFlags() const;
	int getCurrentLangCodeIdx( ) const;
	QStringList getAvailableLanguages( bool withNames=true, LanguageListMode mode=ShowInEnglish ) const;
private:	

	QMap<int, QString> _langCodeIdx;
	QString _langFile;
	QMap<QString, QPixmap> _langFlags;
	TranslationManager *_currentTranslator;

};

#endif