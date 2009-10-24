/***************************************************************************
 *   Copyright (C) 2009 by Dominik Kapusta            <d@ayoy.net>         *
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


#include "urlshortener.h"
#include "urlshortenerimplementation.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QRegExp>
#include <QStringList>

UrlShortener::UrlShortener( QObject *parent ) :
        QObject( parent ),
        shortenerInstance( 0 )
{}

QMap<QString,int> UrlShortener::shorteners() const
{
    QMap<QString,int> map;
    map.insert( "bit.ly",      ShortenerBitly );
    map.insert( "Boooom!",     ShortenerBooom );
    map.insert( "Digg",        ShortenerDigg );
    map.insert( "is.gd",       ShortenerIsgd );
    map.insert( "MetaMark",    ShortenerMetamark );
    map.insert( "Migre.me",    ShortenerMigreme );
    map.insert( "tinyarro.ws", ShortenerTinyarrows );
    map.insert( "TinyURL",     ShortenerTinyurl );
    map.insert( "tr.im",       ShortenerTrim );
    map.insert( "u.nu",        ShortenerUnu );
    return map;
}

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
    case UrlShortener::ShortenerTrim:
        shortenerInstance = new TrimShortener( this );
        break;
    case UrlShortener::ShortenerMetamark:
        shortenerInstance = new MetamarkShortener( this );
        break;
    case UrlShortener::ShortenerTinyurl:
        shortenerInstance = new TinyurlShortener( this );
        break;
    case UrlShortener::ShortenerBooom:
        shortenerInstance = new BoooomShortener( this );
        break;
    case UrlShortener::ShortenerTinyarrows:
        shortenerInstance = new TinyarrowsShortener( this );
        break;
    case UrlShortener::ShortenerUnu:
        shortenerInstance = new UnuShortener( this );
        break;
    case UrlShortener::ShortenerBitly:
        shortenerInstance = new BitlyShortener( this );
        break;
    case UrlShortener::ShortenerDigg:
        shortenerInstance = new DiggShortener( this );
        break;
    case UrlShortener::ShortenerMigreme:
        shortenerInstance = new MigremeShortener( this );
        break;
    case UrlShortener::ShortenerIsgd:
    default:
        shortenerInstance = new IsgdShortener( this );
    }
    connect( shortenerInstance, SIGNAL(shortened(QString,QString)), this, SIGNAL(shortened(QString,QString)) );
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

/*! \class BoooomShortener
    \brief This class is responsible for interacting with http://b.oooom.net
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

