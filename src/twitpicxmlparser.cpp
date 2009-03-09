/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include <QXmlAttributes>
#include "twitpicxmlparser.h"

const QString TwitPicXmlParser::TAG_RESPONSE        = "rsp";
const QString TwitPicXmlParser::TAG_RESPONSE_STATUS = "status";
const QString TwitPicXmlParser::TAG_RESPONSE_OK     = "ok";
const QString TwitPicXmlParser::TAG_RESPONSE_NOK    = "fail";
const QString TwitPicXmlParser::TAG_STATUS_ID       = "statusid";
const QString TwitPicXmlParser::TAG_MEDIA_URL       = "mediaurl";
const QString TwitPicXmlParser::TAG_ERROR           = "err";
const QString TwitPicXmlParser::TAG_ERROR_MESSAGE   = "msg";

TwitPicXmlParser::TwitPicXmlParser( QObject *parent ) :
    QObject( parent ),
    success( false ),
    newStatus( false ),
    urlIncoming( false )
{}

bool TwitPicXmlParser::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName, const QXmlAttributes &atts )
{
  if ( qName == TAG_RESPONSE ) {
    if ( atts.value( TAG_RESPONSE_STATUS ) == TAG_RESPONSE_OK )
      success = true;
    else
      success = false;
    return true;
  }
  if ( !success ) {
    if ( qName == TAG_ERROR ) {
      emit completed( false, atts.value( TAG_ERROR_MESSAGE ), false );
      return true;
    }
  }
  if ( qName == TAG_MEDIA_URL ) {
    urlIncoming = true;
  }
  if ( qName == TAG_STATUS_ID ) {
    newStatus = true;
  }
  return true;
}

bool TwitPicXmlParser::characters( const QString &ch )
{
  if ( success && urlIncoming ) {
    emit completed( true, ch.toAscii(), newStatus );
  }
  return true;
}
