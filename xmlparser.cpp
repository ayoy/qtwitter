#include "xmlparser.h"
#include "mainwindow.h"
#include "entry.h"

#include <QtDebug>

XmlParser::XmlParser() :
  QXmlDefaultHandler(),
  lastTag( "" ),
  spaces( "\\s*" ),
  important(false)
  {}

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
  if ( qName.compare("text") == 0 || qName.compare("name") == 0 || qName.compare("profile_image_url") == 0 ) {
    important = true;
  }
  else {
    important = false;
  }
  
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
  if ( !spaces.exactMatch(ch) && important == true ) {
    important == false;
    qDebug() << ch;
    emit dataParsed( ch );
  }
  return true;
}

