#include "xmlparser.h"

#include <QtDebug>

bool XmlParser::startDocument() {
  qDebug() << "Start of document";
  return true;
}

bool XmlParser::endDocument() {
  qDebug() << "End of document";
  return true;
}

bool XmlParser::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts ) {
  qDebug() << "Start of element" << qName;
  for( int i = 0; i<atts.length(); ++i ) {
    qDebug() << " " << atts.qName(i) << "=" << atts.value(i);
  }
  
  return true;
}

bool XmlParser::endElement( const QString &namespaceURI, const QString &localName, const QString &qName ) {
  qDebug() << "End of element" << qName;
  return true;
}

bool XmlParser::characters( const QString &ch ) {
  if ( ch.length() > 0 ) {
    qDebug() << ch;
  }
  return true;
}

