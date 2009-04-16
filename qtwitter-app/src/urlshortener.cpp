/***************************************************************************
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QRegExp>
#include "urlshortener.h"

UrlShortener::UrlShortener( QObject *parent ) : QObject( parent )
{
    manager = new QNetworkAccessManager( this );
    connect( manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
    connect( this, SIGNAL(errorMessage(QString)), parent, SIGNAL(errorMessage(QString)) );
}

int UrlShortener::replyStatus( QNetworkReply *reply ) const
{
  return reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
}

UrlShortener::~UrlShortener() {}

IsGdShortener::IsGdShortener( QObject *parent ) : UrlShortener( parent ) {}

void IsGdShortener::shorten( const QString &url )
{
  QRegExp rx("http://is.gd/");
  if( rx.indexIn( url ) == -1 ){
    manager->get( QNetworkRequest( QUrl( "http://is.gd/api.php?longurl=" + url ) ) );
  }
}

void IsGdShortener::replyFinished( QNetworkReply * reply )
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
          emit errorMessage( tr( "The URL entered is a potential spam site and is listed on either the SURBL or URIBL blacklist" ) );
        } else if ( message == "The URL you entered is on our blacklist (links to URL shortening sites or is.gd itself are disabled to prevent misuse)" ) {
          emit errorMessage( tr( "The URL you entered is on our blacklist (links to URL shortening sites or is.gd itself are disabled to prevent misuse)" ) );
        } else if ( message == "The address making this request has been blocked by is.gd (normally the result of a violation of our terms of use)" ) {
          emit errorMessage( tr( "The address making this request has been blocked by is.gd (normally the result of a violation of our terms of use)" ) );
        }
      }
      break;
    default:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}

TrImShortener::TrImShortener( QObject *parent ) : UrlShortener( parent ) {}

void TrImShortener::shorten( const QString &url )
{
  QString newUrl =  url.indexOf("http://") > -1 ? url : "http://" + url;
  QRegExp rx("http://tr.im/");
  if( rx.indexIn( newUrl ) == -1 ){
    manager->get( QNetworkRequest( QUrl( "http://api.tr.im/api/trim_simple?url=" + newUrl ) ) );
  }
}

void TrImShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    case 401:
      emit errorMessage( tr( "Submitted URL is invalid." ) );
      break;
    case 402:
      errorMessage( tr( "Submitted URL is already a shortened URL." ) );
      break;
    case 403:
      errorMessage( tr( "The URL has been Flagged as Spam and Rejected." ) );
      break;
    default: case 450:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}

MetaMarkShortener::MetaMarkShortener( QObject *parent ) : UrlShortener( parent ) {}

void MetaMarkShortener::shorten( const QString &url )
{
  QRegExp rx("http://xrl.us/");
  if( rx.indexIn( url ) == -1 ){
    manager->get( QNetworkRequest( QUrl( "http://metamark.net/api/rest/simple?long_url=" + url ) ) );
  }
}

void MetaMarkShortener::replyFinished( QNetworkReply *reply )
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

/*! \class UrlShortener
    \brief A class responsible for interacting with URL shortening services.

    This class provides an interface for communicating with URL shortening services.
*/

/*! \fn UrlShortener::UrlShortener( QObject *parent )
    Creates a new instance of UrlShorten class with the given \a parent
*/

/*! \fn UrlShortener::~UrlShortener()
    Destroys UrlShorten instance
*/

/*! \fn void UrlShortener::shorten( const QString &url )
    Sends a request to the shortening service with the give \a url
*/

/*! \fn void UrlShortener::shortened( const QString &url )
    Emitted for a shortened URL
*/

