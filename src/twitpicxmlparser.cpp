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


#include <QXmlAttributes>
#include <QDebug>
#include "twitpicxmlparser.h"

const QString TwitPicXmlParser::TAG_RESPONSE         = "rsp";
const QString TwitPicXmlParser::TAG_RESPONSE_STATUS  = "stat";
const QString TwitPicXmlParser::TAG_RESPONSE_STATUS2 = "status";
const QString TwitPicXmlParser::TAG_RESPONSE_OK      = "ok";
const QString TwitPicXmlParser::TAG_RESPONSE_NOK     = "fail";
const QString TwitPicXmlParser::TAG_STATUS_ID        = "statusid";
const QString TwitPicXmlParser::TAG_MEDIA_URL        = "mediaurl";
const QString TwitPicXmlParser::TAG_ERROR            = "err";
const QString TwitPicXmlParser::TAG_ERROR_CODE       = "code";
const QString TwitPicXmlParser::TAG_ERROR_MESSAGE    = "msg";

TwitPicXmlParser::TwitPicXmlParser( QObject *parent ) :
    QObject( parent ),
    success( false ),
    newStatus( false ),
    urlIncoming( false )
{}

bool TwitPicXmlParser::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName, const QXmlAttributes &atts )
{
  qDebug() << "qName:" << qName << success;
  if ( qName == TAG_RESPONSE ) {
    qDebug() << atts.qName( 0 ) << atts.value( atts.qName( 0 ) );
    if ( atts.value( TAG_RESPONSE_STATUS ) == TAG_RESPONSE_OK || atts.value( TAG_RESPONSE_STATUS2 ) == TAG_RESPONSE_OK )
      success = true;
    else
      success = false;
    return true;
  }
  if ( !success ) {
    if ( qName == TAG_ERROR ) {
      qDebug() << "ERROR:" << atts.value( TAG_ERROR_CODE ) << atts.value( TAG_ERROR_MESSAGE );
      emit completed( false, atts.value( TAG_ERROR_MESSAGE ), false );
      return true;
    }
  }
  if ( qName == TAG_MEDIA_URL ) {
    qDebug() << "urlIncoming = true";
    urlIncoming = true;
  }
  if ( qName == TAG_STATUS_ID ) {
    qDebug() << "newStatus = true";
    newStatus = true;
  }
  return true;
}

bool TwitPicXmlParser::characters( const QString &ch )
{
  if ( QRegExp( "\\s*" ).exactMatch( ch ) )
    return true;
  if ( success && urlIncoming ) {
    qDebug() << "ch:" << ch << "newStatus:" << newStatus;
    emit completed( true, ch.toAscii(), newStatus );
  }
  return true;
}
