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
  entry(),
  important( false ),
  entryComplete( false )
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
    entry.setIndex( entry.getIndex() + 1 );
  }
//  for( int i = 0; i<atts.length(); ++i ) {
//    qDebug() << " " << atts.qName(i) << "=" << atts.value(i);
//  }
  return true;
}

bool XmlParser::endElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &/*qName*/ ) {
  //qDebug() << "End of element" << qName;
  return true;
}

bool XmlParser::characters( const QString &ch ) {
  if ( important ) {
    if ( lastField == Id && entry.id() == -1 ) {
      entry.setId( ch.toInt() );
//      qDebug() << "Setting id  with: " << ch;
    } else if ( lastField == Name && !entry.name().compare( "" ) ) {
      entry.setName( ch );
//      qDebug() << "Setting name  with: " << ch;
    } else if ( lastField == Login && !entry.login().compare( "" ) ) {
      entry.setLogin( ch );
//      qDebug() << "Setting login  with: " << ch;
    } else if ( lastField == Text && !entry.text().compare( "" ) ) {
      entry.setText( ch );
//      qDebug() << "Setting text  with: " << ch;
    } else if ( lastField == Image && !entry.image().compare( "" ) ) {
      entry.setImage( ch );
//      qDebug() << "Setting image with: " << ch;
    } else if ( lastField == Homepage ) {
      entryComplete = true;
      if ( !QRegExp( "\\s*" ).exactMatch( ch ) ) {
        entry.setHasHomepage( true );
        entry.setHomepage( ch );
        qDebug() << "Setting homepage with: " << ch;
      }
    }
    if ( entryComplete && entry.checkContents() ) {
      emit newEntry( &entry );
      lastField = None;
      entryComplete = false;
    }
    important = false;
  }
  return true;
}

int XmlParser::checkFieldType(const QString &element ) {
  if ( !element.compare(USER_ID) )
    return Id;
  if ( !element.compare(USER_STATUS) )
    return Text;
  if ( !element.compare(USER_NAME) )
    return Name;
  if ( !element.compare(USER_LOGIN) )
    return Login;
  if ( !element.compare(USER_HOMEPAGE) )
    return Homepage;
  if ( !element.compare(USER_PHOTO) )
    return Image;
  if ( !element.compare( "status" ) ) {
    entry.setId( -1 );
    entry.setName( "" );
    entry.setOwn( false );
    entry.setLogin( "" );
    entry.setHomepage( "" );
    entry.setHasHomepage( false );
    entry.setText( "" );
    entry.setImage( "" );
  }
  return None;
}
