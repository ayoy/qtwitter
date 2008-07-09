#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QtXml>
#include <QUrl>
#include <QObject>
#include <QRegExp>

class XmlParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT

private:
  QString message;  
  QString lastTag;
  QUrl *avatar;
  QString *status;
  QRegExp spaces;
  bool important;

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

signals:
  void dataParsed( const QString &text );
  void newEntry( const QUrl &avatar, const QString &status );

};

#endif //XMLPARSER_H