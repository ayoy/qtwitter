/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "xmlparser.h"

XmlParser::XmlParser() :
  QXmlDefaultHandler(),
  lastField( None ),
  type( All ),
  entry(),
  important( false )
  {
  }

XmlParser::XmlParser( XmlType type ) :
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
  if ( lastField == Text ) {
    entry.setId( entry.getId() + 1 );
  }
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
