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

const QByteArray XmlParser::USER_ID = "id";
const QByteArray XmlParser::USER_TEXT = "text";
const QByteArray XmlParser::USER_NAME = "name";
const QByteArray XmlParser::USER_LOGIN = "screen_name";
const QByteArray XmlParser::USER_PHOTO = "profile_image_url";
const QByteArray XmlParser::USER_HOMEPAGE = "url";
const QByteArray XmlParser::USER_TIMESTAMP = "created_at";

XmlParser::XmlParser( QObject *parent) :
  QObject( parent ),
  QXmlDefaultHandler(),
  currentField( None ),
  entry(),
  important( false )
  {
  }

XmlParser::XmlParser( Entry::Type entryType, QObject *parent) :
  QObject( parent ),
  QXmlDefaultHandler(),
  currentField( None ),
  entry( entryType ),
  important( false )
  {
  }

bool XmlParser::startDocument() {
  return true;
}

bool XmlParser::endDocument() {
  emit xmlParsed();
  return true;
}

bool XmlParser::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName, const QXmlAttributes & /*atts*/ ) {
  if ( qName == "status" ) {
    entry.initialize();
    entry.setIndex( entry.getIndex() + 1 );
  }
  ( (currentField = checkFieldType( qName )) != None ) ? important = true : important = false;
  return true;
}

bool XmlParser::endElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName ) {
  if ( qName == "status" ) {
    emit newEntry( &entry );
  }
  return true;
}

bool XmlParser::characters( const QString &ch ) {
  if ( important ) {
    if ( currentField == Id && entry.id() == -1 ) {
      entry.setId( ch.toInt() );
//      qDebug() << "Setting id  with: " << ch;
    } else if ( currentField == Name && entry.name().isNull() ) {
      entry.setName( ch );
//      qDebug() << "Setting name  with: " << ch;
    } else if ( currentField == Login && entry.login().isNull() ) {
      entry.setLogin( ch );
//      qDebug() << "Setting login  with: " << ch;
    } else if ( currentField == Text && entry.text().isNull() ) {
      entry.setText( ch );
//      qDebug() << "Setting text  with: " << ch;
    } else if ( currentField == Image && entry.image().isNull() ) {
      entry.setImage( ch );
//      qDebug() << "Setting image with: " << ch;
    } else if ( currentField == Timestamp && entry.timestamp().isNull() ) {
      entry.setTimestamp( toDateTime( ch ) );
//      qDebug() << "Setting timestamp with: " << ch;
    } else if ( currentField == Homepage ) {
      if ( !QRegExp( "\\s*" ).exactMatch( ch ) ) {
        entry.setHasHomepage( true );
        entry.setHomepage( ch );
//        qDebug() << "Setting homepage with: " << ch;
      }
    }
  }
  return true;
}

int XmlParser::getMonth( const QString &month )
{
  if ( month == "Jan" )
    return 1;
  if ( month == "Feb" )
    return 2;
  if ( month == "Mar" )
    return 3;
  if ( month == "Apr" )
    return 4;
  if ( month == "May" )
    return 5;
  if ( month == "Jun" )
    return 6;
  if ( month == "Jul" )
    return 7;
  if ( month == "Aug" )
    return 8;
  if ( month == "Sep" )
    return 9;
  if ( month == "Oct" )
    return 10;
  if ( month == "Nov" )
    return 11;
  if ( month == "Dec" )
    return 12;
  else
    return -1;
}

QDateTime XmlParser::toDateTime( const QString &timestamp ) {
  QRegExp rx( "(\\w+) (\\w+) (\\d\\d) (\\d\\d):(\\d\\d):(\\d\\d) .+ (\\d\\d\\d\\d)" );
  rx.indexIn( timestamp );
  return QDateTime( QDate( rx.cap(7).toInt(), getMonth( rx.cap(2) ), rx.cap(3).toInt() ), QTime( rx.cap(4).toInt(), rx.cap(5).toInt(), rx.cap(6).toInt() ) );
}

XmlParser::FieldType XmlParser::checkFieldType(const QString &element ) {
  if ( !element.compare(USER_ID) )
    return Id;
  if ( !element.compare(USER_TEXT) )
    return Text;
  if ( !element.compare(USER_NAME) )
    return Name;
  if ( !element.compare(USER_LOGIN) )
    return Login;
  if ( !element.compare(USER_HOMEPAGE) )
    return Homepage;
  if ( !element.compare(USER_PHOTO) )
    return Image;
  if ( !element.compare(USER_TIMESTAMP) )
    return Timestamp;
  return None;
}
