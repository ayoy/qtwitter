#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QtXml>
#include <QUrl>
#include <QObject>

#include "entry.h"

#define USER_STATUS "text"
#define USER_LOGIN "name"
#define USER_PHOTO "profile_image_url"

class XmlParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT
  
private:
  int lastField;
  int type;
  Entry entry;
  bool important;
  int checkFieldType( const QString &element );
  
public:
  XmlParser();
  XmlParser( int type );
  
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

  enum FieldType {
    None,
    Name,
    Image,
    Text    
  };
  
  enum XmlType {
    One,
    All
  };

signals:
  void dataParsed( const QString &text );
  void newEntry( const Entry &entry, int type );
  void xmlParsed();

};

#endif //XMLPARSER_H
