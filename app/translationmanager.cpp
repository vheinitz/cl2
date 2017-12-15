#include "translationmanager.h"
#include <QTextStream>
#include <QFile>
#include <QDomDocument>
#include <QSet>


bool TranslationManager::readLangData(QString csv)
{
	QFile csvf(csv);
	if (! csvf.open(QIODevice::ReadOnly) )
		return false;
	QTextStream ts( &csvf );
	_header = ts.readLine().split(";", QString::KeepEmptyParts);
	if ( _header.size() < 2 )
	{
		return false;
	}
	_transData.clear();
	while(!ts.atEnd())
	{
		QString line = ts.readLine();
		if (line.isEmpty())
			continue;

		QStringList entries = line.split(";", QString::KeepEmptyParts);
		if ( entries.size() != _header.size() )
		{
			return false;
		}

		for (int i=1; i<entries.size(); ++i)
		{
			QString phrase = entries.at(i);
			if ( phrase.indexOf("#[+*:OLD:*+]#")==0 )
				phrase.remove( 0, 13 );
			QString lang = _header.at(i);
			if (!lang.isEmpty())
			{
			_transData[ _header.at(i) ][entries.at(0)] = phrase;
		}
	}
	}

    return true;
}

QString TranslationManager::translate ( const char * context, const char * sourceText, const char * disambiguation ) const
{
	
	/*if ( QString(sourceText).contains("Workl")   )
	{
		int i=0;
		i++;
	}*/

	if (!_transData.keys().contains( _currentLanguage ) )
	{
		//TODO WARNING unsupported lang
		return sourceText;
	}

	else if (!_transData[_currentLanguage].contains( sourceText ) )
	{
		//TODO WARNING untranslated text
		return sourceText;
	}
	
	return _transData[_currentLanguage][sourceText];

}


///the language table should be loaded before
bool TranslationManager::setCurrentLanguage( QString cl )
{
	 if ( _transData.isEmpty() || !_transData.keys().contains( cl ) )
	 {
		return false;
	 }
	_currentLanguage = cl;
	return true;
}

bool TranslationManager::updateLangData(QString fname)
{
	QFile sourceFile ( fname );
	QDomDocument qts;
	if (!qts.setContent(&sourceFile)) 
		return false;
	QDomNodeList dnl = qts.elementsByTagName("source");
	int phnum = dnl.size();
	QSet<QString> old_phrases = _transData.begin().value().keys().toSet(); 

	QSet<QString> act_phrases; 
	for( int i=0; i<dnl.size(); ++i )
	{
		act_phrases.insert( dnl.at(i).toElement().text() );
	}

	QSet<QString> tmp = act_phrases;
	QSet<QString> newPhrases = tmp.subtract( old_phrases );

	tmp = old_phrases;
	QSet<QString> obsoletePhrases = tmp.subtract( act_phrases );
	


	TLangData tmpTD = _transData;
	for ( TLangData::iterator lit = _transData.begin(); lit != _transData.end(); ++lit )
	{

		TLangTransData tmpLangTD;
		if ( lit.value().isEmpty() )
		{
			return false; //invalid format
		}


		QStringList phrases = lit.value().keys();
		foreach( QString ph, phrases )
		{
			if ( !newPhrases.contains(ph) )
			{
				tmpLangTD[ QString("#[+*:OLD:*+]#%1").arg( ph ) ] = lit.value()[ph];
			}
		}


		foreach(QString ph,newPhrases)
			tmpLangTD[ph]=ph;
		tmpTD.insert(lit.key(),tmpLangTD);
	}

	_transData = tmpTD;
	
    return true;
}

bool TranslationManager::saveCsv(QString csv)
{
	if ( _transData.isEmpty() )
	{
		return false; //invalid format
	}
	
	QFile csvf(csv);
	csvf.open(QIODevice::WriteOnly);
	QTextStream ts( &csvf );
	foreach(QString hi,_header) ts <<hi<<";";
	ts <<"\n";

	QStringList phrases = _transData.begin().value().keys(); 

	foreach( QString ph, phrases )
	{
		ts << ph <<";";
		for ( TLangData::iterator lit = _transData.begin(); lit != _transData.end(); ++lit )
		{
			ts <<lit.value()[ ph ] <<";";
		}
		ts <<"\n";
	}
    return false;
}