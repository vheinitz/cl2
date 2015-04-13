#include "xml/xpath.h"
#include <QFile>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDomDocument>

int XPath::xpathCount( QDomDocument &doc, QString xpath )
{
	QStringList nodes = xpath.split("/");
	QDomElement xne = doc.elementsByTagName(nodes.at(0)).at(0).toElement();
	
	for(int i=1; i< nodes.size()-1; ++i )
	{
		QString sn = nodes.at(i).section("[",0,0);
		int idx = nodes.at(i).section("[",1).section("]",0,0).toInt();
		{
			xne = xne.elementsByTagName(sn).at(idx).toElement();								
		}								
	}				
	return xne.elementsByTagName(nodes.last()).size();
}

//TODO: fails when querying for nameA/nameA/... - parent and child same name
QString XPath::xpathValue( QDomDocument &doc, QString xpath )
{
	QString value;
	QStringList nodes = xpath.split("/");
	QDomElement xne = doc.elementsByTagName(nodes.at(0)).at(0).toElement();
	
	for(int i=1; i< nodes.size(); ++i )
	{
		QString sn = nodes.at(i).section("[",0,0);
		int idx = nodes.at(i).section("[",1).section("]",0,0).toInt();
		QDomNodeList childs = xne.childNodes();
		int lastFoundIdx=-1;
		bool found=false;
		for(int chIdx=0; chIdx<childs.size(); ++chIdx)
		{
			QDomNode child = childs.at(chIdx);
			QString chName = child.toElement().tagName();
			if (chName == sn )
			{
				lastFoundIdx ++;
			}

			if (lastFoundIdx == idx )
			{
				xne = child.toElement();//xne.elementsByTagName(sn).at(idx).toElement();	
				found=true;
				break;
			}
		}
		if (!found)
		{
			return QString::null;
		}
			
	}			
	value = xne.text();
	return value;
}

//TODO: fails when querying for nameA/nameA/... - parent and child same name
QString XPath::xpathAttrValue( QDomDocument &doc, QString xpath )
{
	QString value;
	QStringList nodes = xpath.split("/");
	QDomElement xne = doc.elementsByTagName(nodes.at(0)).at(0).toElement();
	
	for(int i=1; i< nodes.size()-1; ++i )
	{
		QString sn = nodes.at(i).section("[",0,0);
		int idx = nodes.at(i).section("[",1).section("]",0,0).toInt();
		QDomNodeList childs = xne.childNodes();
		int lastFoundIdx=-1;
		bool found=false;
		for(int chIdx=0; chIdx<childs.size(); ++chIdx)
		{
			QDomNode child = childs.at(chIdx);
			QString chName = child.toElement().tagName();
			if (chName == sn )
			{
				lastFoundIdx ++;
			}

			if (lastFoundIdx == idx )
			{
				xne = child.toElement();//xne.elementsByTagName(sn).at(idx).toElement();	
				found=true;
				break;
			}
		}
		if (!found)
		{
			return QString::null;
		}
			
	}

	value = xne.attributeNode(nodes.last()).value();
	return value;
}

QDomElement XPath::xpathElement( QDomDocument &doc, QString xpath )
{
	QStringList nodes = xpath.split("/");
	QDomElement xne = doc.elementsByTagName(nodes.at(0)).at(0).toElement();
	if(xne.isNull())
		return xne;

	for(int i=1; i< nodes.size(); ++i )
	{
		QString sn = nodes.at(i).section("[",0,0);
		int idx = nodes.at(i).section("[",1).section("]",0,0).toInt();
		xne = xne.elementsByTagName(sn).at(idx).toElement();												
		if(xne.isNull())
			return xne;
	}			

	return xne;
}
