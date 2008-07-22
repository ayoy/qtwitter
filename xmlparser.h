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
  QString message;  
  QUrl *image;
  QString *status;
  int lastField;
  Entry entry;
  bool important;
  int checkFieldType( const QString &element );


public:
  XmlParser();
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

signals:
  void dataParsed( const QString &text );
  void newEntry( const Entry &entry );

};

#endif //XMLPARSER_H
