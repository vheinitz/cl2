#include <QtTest/QtTest>
#include <QDomDocument>
#include <../xml/xpath.h>

class test_xml: public QObject
{
    Q_OBJECT
	QDomDocument _dom;
private slots:
	void initTestCase();
	void test_count();
	void test_value();
	void test_attr();
	void test_element();
};
 
QString xmlData = 
"<?xml version=\"1.0\"?>"
"<!DOCTYPE PARTS SYSTEM \"parts.dtd\">"
"<?xml-stylesheet type=\"text/css\" href=\"xmlpartsstyle.css\"?>"
"<PARTS>"
"   <TITLE>Computer Parts</TITLE>"
"   <PART>"
"      <ITEM>Motherboard</ITEM>"
"      <MANUFACTURER>ASUS</MANUFACTURER>"
"      <MODEL>P3B-F</MODEL>"
"      <COST currency=\"EUR\"> 123.00</COST>"
"   </PART>"
"   <PART>"
"      <ITEM>Video Card</ITEM>"
"      <MANUFACTURER>ATI</MANUFACTURER>"
"      <MODEL>All-in-Wonder Pro</MODEL>"
"      <COST currency=\"USD\"> 160</COST>"
"   </PART>"
"</PARTS>";


void test_xml::initTestCase()
{
	QVERIFY( _dom.setContent( xmlData ) == true );
}

void test_xml::test_count()
{
	QVERIFY( XPath::xpathCount(_dom, "PARTS/PART") == 2);
	QVERIFY( XPath::xpathCount(_dom, "PARTS/TITLE") == 1);
	QVERIFY( XPath::xpathCount(_dom, "PARTS/NoSuchTag") == 0);

}

void test_xml::test_value()
{
	QVERIFY( XPath::xpathValue(_dom, "PARTS/TITLE") == "Computer Parts");

	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[0]/ITEM") == "Motherboard");
	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[0]/MANUFACTURER") == "ASUS");
	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[0]/MODEL") == "P3B-F");
	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[0]/COST").toFloat() == 123.0);

	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[1]/ITEM") == "Video Card");
	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[1]/MANUFACTURER") == "ATI");
	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[1]/MODEL") == "All-in-Wonder Pro");
	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[1]/COST").toInt() == 160 );

	QVERIFY( XPath::xpathValue(_dom, "PARTS/PART[2]/COST").isEmpty() );

}

void test_xml::test_attr()
{
	QVERIFY( XPath::xpathAttrValue(_dom, "PARTS/PART[0]/COST/currency") == "EUR" );
	QVERIFY( XPath::xpathAttrValue(_dom, "PARTS/PART[1]/COST/currency") == "USD" );
}

void test_xml::test_element()
{
	QVERIFY( !XPath::xpathElement(_dom, "TITLE").isNull() );
	QVERIFY( !XPath::xpathElement(_dom, "PARTS/TITLE").isNull() );
	QVERIFY( XPath::xpathElement(_dom, "NoSuchTag").isNull() );
	QVERIFY( XPath::xpathElement(_dom, "PARTS/PART[0]").isElement() );	
}


QTEST_MAIN(test_xml)
#include "test_xml.moc"