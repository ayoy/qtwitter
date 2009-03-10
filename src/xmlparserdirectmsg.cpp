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


#include "xmlparserdirectmsg.h"

XmlParserDirectMsg::XmlParserDirectMsg( QObject *parent ) :
    XmlParser( Entry::DirectMessage, parent ),
    parsingSender( false )
{}

bool XmlParserDirectMsg::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName, const QXmlAttributes & /*atts*/ )
{
  if ( qName == "direct_message" ) {
    entry.initialize();
    entry.setIndex( entry.getIndex() + 1 );
  }
  if ( qName == "sender" ) {
    parsingSender = true;
  }
  ( (currentField = checkFieldType( qName )) != None ) ? important = true : important = false;
  return true;
}

bool XmlParserDirectMsg::endElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName )
{
  if ( qName == "direct_message" ) {
    emit newEntry( &entry );
  }
  if ( qName == "sender" ) {
    parsingSender = false;
  }
  return true;
}

bool XmlParserDirectMsg::characters( const QString &ch )
{
  if ( important ) {
    if ( currentField == Id && entry.id() == -1 ) {
      entry.setId( ch.toInt() );
    } else if ( currentField == Text && entry.text().isNull() ) {
      entry.setText( ch );
    } else if ( currentField == Timestamp && entry.timestamp().isNull() ) {
      entry.setTimestamp( toDateTime( ch ) );
    }
    if ( parsingSender ) {
      if ( currentField == Name && entry.name().isNull() ) {
        entry.setName( ch );
      } else if ( currentField == Login && entry.login().isNull() ) {
        entry.setLogin( ch );
      } else if ( currentField == Homepage ) {
        if ( !QRegExp( "\\s*" ).exactMatch( ch ) ) {
          entry.setHasHomepage( true );
          entry.setHomepage( ch );
        }
      }
    }
  }
  return true;
}
