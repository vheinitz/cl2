#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QMap>
#include <QList>
#include <QTranslator>
#include <QStringList>
#include <cldef.h>

typedef QString TLanguageName;
typedef QString TOrigLangWord;
typedef QString TTransLangWord;

typedef QMap<TOrigLangWord,TTransLangWord> TLangTransData;
typedef QMap<TLanguageName,TLangTransData> TLangData;


class CLIB_EXPORT TranslationManager : public QTranslator
{
	
	TLangData _transData;
	QStringList _header;
	QString _currentLanguage;
public:
	TranslationManager( QObject *p):QTranslator(p){}
	

	bool readLangData(QString); // read csv file containing entire translation

    /// read Qt ts file and update orig entries
    ///
    bool updateLangData(QString);

    bool saveCsv(QString);

	bool setCurrentLanguage( QString );
	
	QStringList availableLanguages() const 
	{
		if ( _header.isEmpty() )
			return QStringList();
		else
			return _header.filter(QRegExp("^..$"));//TODO more robust way
	}

	void clear() { _transData.clear(); _header.clear(); }

	QString translate ( const char * context, const char * sourceText, const char * disambiguation = 0 ) const;

    
};

#endif // TRANSLATIONMANAGER_H
