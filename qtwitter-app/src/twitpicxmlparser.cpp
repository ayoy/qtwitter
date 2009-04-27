/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
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


/*! \class TwitPicXmlParser
    \brief A class for parsing XML data from TwitPic API.

    This class inherits QXmlDefaultHandler and reimplements its methods to comply
    with the structure of XML documents provided by TwitPic REST API.
*/

/*! \fn TwitPicXmlParser::TwitPicXmlParser( QObject *parent = 0 )
    Creates a new parser with a given \a parent.
*/

/*! \fn virtual bool TwitPicXmlParser::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts )
  Parser activities at start of the XML element.
*/

/*! \fn virtual bool TwitPicXmlParser::characters( const QString &ch )
  Parser activities when reading XML element's text. The actual values are
  being read here.
*/

/*! \fn void TwitPicXmlParser::completed( bool responseStatus, QString message, bool newStatus )
    Emitted when full response is parsed and ready to be processed.
    \param responseStatus Indicates whether request completed successfully or failed.
    \param message Depending on the value of \a responseStatus, contains an error message when false or a URL link to photo when true.
    \param newStatus Indicates whether a new status has been posted to Twitter.
*/
