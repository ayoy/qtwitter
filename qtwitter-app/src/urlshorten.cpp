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
#include "urlshorten.h"

UrlShorten::UrlShorten( QObject *parent ) : QObject( parent )
{
    manager = new QNetworkAccessManager( this );
    connect( manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
    connect( this, SIGNAL(errorMessage(QString)), parent, SIGNAL(errorMessage(QString)) );
}

UrlShorten::~UrlShorten()
{
}

void UrlShorten::shorten( const QString &url )
{
  QRegExp rx("http://is.gd/");
  if( rx.indexIn( url ) == -1 ){
    manager->get( QNetworkRequest( QUrl( "http://is.gd/api.php?longurl=" + url ) ) );
  }
}

void UrlShorten::replyFinished( QNetworkReply * reply )
{
  int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QString response = reply->readLine();

  switch( status ) {
    case 200:
      emit shortened( response );
      break;
    case 500:
      emit errorMessage( response.replace("Error: ", "") );
      break;
    default:
      emit errorMessage( "An unknown error occurred when shortening your URL" );
  }
}

/*! \class UrlShorten
    \brief A class responsible for interacting with URL shortening services.

    This class provides an interface for communicating with URL shortening services.
    Right now, it only supports http://is.gd
*/

/*! \fn UrlShorten::UrlShorten( QObject *parent )
    Creates a new instance of UrlShorten class with the given \a parent
*/

/*! \fn UrlShorten~UrlShorten()
    Destroys UrlShorten instance
*/

/*! \fn void UrlShorten::shorten( const QString &url )
    Sends a request to the shortening service with the give \a url
*/

/*! \fn void UrlShorten::shortened( const QString &url )
    Emitted for a shortened URL
*/

