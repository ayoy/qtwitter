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
#include "urlshortenerimplementation.h"

UrlShortener::UrlShortener( QObject *parent ) :
    QObject( parent ),
    shortenerInstance( 0 )
{}

void UrlShortener::shorten( const QString &url, UrlShortener::Shortener shorteningService )
{
  if ( shortenerInstance ) {
    if ( shortenerInstance->shorteningService() == shorteningService ) {
      shortenerInstance->shorten( url );
      return;
    }
    delete shortenerInstance;
  }

  switch( shorteningService ) {
  case UrlShortener::SHORTENER_TRIM:
    shortenerInstance = new TrimShortener( this );
    break;
  case UrlShortener::SHORTENER_METAMARK:
    shortenerInstance = new MetamarkShortener( this );
    break;
  case UrlShortener::SHORTENER_TINYURL:
    shortenerInstance = new TinyurlShortener( this );
    break;
  case UrlShortener::SHORTENER_TINYARROWS:
    shortenerInstance = new TinyarrowsShortener( this );
    break;
  case UrlShortener::SHORTENER_UNU:
    shortenerInstance = new UnuShortener( this );
    break;
  case UrlShortener::SHORTENER_ISGD:
  default:
    shortenerInstance = new IsgdShortener( this );
  }
  connect( shortenerInstance, SIGNAL(shortened(QString)), this, SIGNAL(shortened(QString)) );
  connect( shortenerInstance, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  shortenerInstance->shorten( url );
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

/*! \class UnuShortener
    \brief This class is responsible for interacting with http://u.nu
 */

