#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QtXml>

class XmlParser : public QXmlDefaultHandler
{
public:
  bool startDocument();
  bool endDocument();
  
  bool startElement( const QString &namespaceURI,
                     const QString &localName,
                     const QString &qName,
                     const QXmlAttributes &atts );
  bool endElement( const QString &namespaceURI,
                   const QString &localName,
                   const QString &qName );
                   
  bool characters( const QString &ch );
};

#endif //XMLPARSER_H