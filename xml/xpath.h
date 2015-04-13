#ifndef MY_X_PATH_H
#define MY_X_PATH_H

#include <QString>
#include <QDomDocument>
#include <QDomElement>

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

class CLIB_EXPORT XPath 
{
public:
	static int xpathCount( QDomDocument &doc, QString xpath );
	static QString xpathValue( QDomDocument &doc, QString xpath );
	static QString xpathAttrValue( QDomDocument &doc, QString xpath );
	static QDomElement xpathElement( QDomDocument &doc, QString xpath );
};



#endif // MY_X_PATH_H
