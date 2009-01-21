#include "xmlparser.h"

XmlParser::XmlParser() :
  QXmlDefaultHandler(),
  lastField( None ),
  type( All ),
  entry(),
  important( false )
  {
  }

XmlParser::XmlParser( int type ) :
  QXmlDefaultHandler(),
  lastField( None ),
  type( type ),
  entry(),
  important( false )
  {
  }

bool XmlParser::startDocument() {
  //qDebug() << "Start of document";
  return true;
}

bool XmlParser::endDocument() {
  //qDebug() << "End of document";
  emit xmlParsed();
  return true;
}

bool XmlParser::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName, const QXmlAttributes & /*atts*/ ) {
  ( (lastField = checkFieldType( qName )) != None ) ? important = true : important = false;
  /*for( int i = 0; i<atts.length(); ++i ) {
    qDebug() << " " << atts.qName(i) << "=" << atts.value(i);
  }*/
  return true;
}

bool XmlParser::endElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &/*qName*/ ) {
  //qDebug() << "End of element" << qName;
  return true;
}

bool XmlParser::characters( const QString &ch ) {
  if ( important ) {
    if ( lastField == Name && !entry.name().compare( "" ) ) {
      entry.setName( ch );
      //qDebug() << "Setting name  with: " << ch;
    } else if ( lastField == Text && !entry.text().compare( "" ) ) {
      entry.setText( ch );
      //qDebug() << "Setting text  with: " << ch;
    } else if ( lastField == Image && !entry.image().compare( "" ) ) {
      entry.setImage( ch );
      //qDebug() << "Setting image with: " << ch;
    }
    if ( entry.checkContents() ) {
      emit newEntry( entry, type );
      lastField = None;
    }
    important = false;
  }
  return true;
}

int XmlParser::checkFieldType(const QString &element ) {
  if ( !element.compare(USER_STATUS) )
    return Text;
  if ( !element.compare(USER_LOGIN) )
    return Name;
  if ( !element.compare(USER_PHOTO) )
    return Image;
  if ( !element.compare( "status" ) ) {
    entry.setName( "" );
    entry.setText( "" );
    entry.setImage( "" );
  }
  return None;
}
