/***************************************************************************
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QRegExp>
#include "urlshortener.h"

UrlShortener::UrlShortener( QObject *parent ) : QObject( parent )
{
  connection = new QNetworkAccessManager( this );
  connect( connection, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
  connect( this, SIGNAL(errorMessage(QString)), parent, SIGNAL(errorMessage(QString)) );
}

int UrlShortener::replyStatus( QNetworkReply *reply ) const
{
  return reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
}

UrlShortener::~UrlShortener() {}

IsgdShortener::IsgdShortener( QObject *parent ) : UrlShortener( parent ) {}

void IsgdShortener::shorten( const QString &url )
{
  if( QRegExp( "http://is.gd/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://is.gd/api.php?longurl=" + url ) ) );
  }
}

void IsgdShortener::replyFinished( QNetworkReply * reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    case 500: {
        QString message = response.replace("Error: ", "");
        if( message == "The URL entered was not valid." ) {
          emit errorMessage( tr( "The URL entered was not valid.") );
        } else if ( message == "The URL entered was too long." ) {
          emit errorMessage( tr( "The URL entered was too long.") );
        } else if ( message ==  "The address making this request has been blacklisted by Spamhaus (SBL/XBL) or Spamcop." )  {
          emit errorMessage( tr( "The address making this request has been blacklisted by Spamhaus (SBL/XBL) or Spamcop.") );
        } else if ( message == "The URL entered is a potential spam site and is listed on either the SURBL or URIBL blacklist.") {
          emit errorMessage( tr( "The URL entered is a potential spam site and is listed on either the SURBL or URIBL blacklist." ) );
        } else if ( message == "The URL you entered is on the is.gd's blacklist (links to URL shortening sites or is.gd itself are disabled to prevent misuse)." ) {
          emit errorMessage( tr( "The URL you entered is on the is.gd's blacklist (links to URL shortening sites or is.gd itself are disabled to prevent misuse)." ) );
        } else if ( message == "The address making this request has been blocked by is.gd (normally the result of a violation of its terms of use)." ) {
          emit errorMessage( tr( "The address making this request has been blocked by is.gd (normally the result of a violation of its terms of use)." ) );
        }
      }
      break;
    default:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}

TrimShortener::TrimShortener( QObject *parent ) : UrlShortener( parent ) {}

void TrimShortener::shorten( const QString &url )
{
  QString newUrl = url.indexOf( "http://" ) > -1 ? url : "http://" + url;
  
  if( QRegExp( "http://tr.im/" ).indexIn( newUrl ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://api.tr.im/api/trim_simple?url=" + newUrl ) ) );
  }
}

void TrimShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      if( QRegExp( "\\s*" ).exactMatch( response ) ) {
        emit errorMessage( tr( "The URL has been rejected by the tr.im" ) );
      } else {
        emit shortened( response.trimmed() );
      }
      break;
    default:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}

MetamarkShortener::MetamarkShortener( QObject *parent ) : UrlShortener( parent ) {}

void MetamarkShortener::shorten( const QString &url )
{
  if( QRegExp( "http://xrl.us/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://metamark.net/api/rest/simple?long_url=" + url ) ) );
  }
}

void MetamarkShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    default: case 500:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}

TinyurlShortener::TinyurlShortener( QObject *parent ) : UrlShortener( parent ) {}

void TinyurlShortener::shorten( const QString &url )
{
  if( QRegExp( "http://tinyurl.com/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://tinyurl.com/api-create.php?url=" + url ) ) );
  }
}

void TinyurlShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    default: case 500:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}

TinyarrowsShortener::TinyarrowsShortener( QObject *parent ) : UrlShortener( parent ) {}

void TinyarrowsShortener::shorten( const QString &url )
{
  if( QRegExp( "http://➡.ws/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://tinyarro.ws/api-create.php?utfpure=1&url=" + url ) ) );
  }
}

void TinyarrowsShortener::replyFinished( QNetworkReply *reply )
{
  QString response = QString::fromUtf8( reply->readLine() );

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    default: case 500:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}
/*! \class UrlShortener
    \brief A class responsible for interacting with URL shortering services.

    Provides a basic interface for interacting with URL shortering services.
    Classes which provide handling of particular shortering services should inherit from it.
*/

/*! \fn UrlShortener::UrlShortener( QObject *parent )
    Creates a new instance of UrlShortener class with the given \a parent
    \param parent The object's parent.
*/

/*! \fn virtual UrlShortener::~UrlShortener()
    Destroys UrlShortener instance.
*/

/*! \fn virtual void UrlShortener::shorten( const QString &url )
    Sends a request to the shortening service with the give \a url.
*/

/*! \fn void UrlShortener::shortened( const QString &url )
    Emitted for a shortened URL.
*/

/*! \fn int UrlShortener::replyStatus( QNetworkReply *reply )
    Extracts HTTP status code from the given \a reply.
    \param reply Network reply
    \returns HTTP status code
*/

/*! \fn virtual void UrlShortener::replyFinished( QNetworkReply *reply )
    Called when the request is finished, it processes the reply and emits appropriate signals.
    \param reply Network reply
*/

/*! \class IsgdShortener
    \brief This class is responsible for interacting with http://is.gd
 */

/*! \class TrimShortener
    \brief This class is responsible for interacting with http://tr.im
 */

/*! \class MetamarkShortener
    \brief This class is responsible for interacting with http://metamark.com
 */

/*! \class TinyurlShortener
    \brief This class is responsible for interacting with http://tinyurl.com
 */

/*! \class TinyarrowsShortener
    \brief This class is responsible for interacting with http://tinyarro.ws
 */
