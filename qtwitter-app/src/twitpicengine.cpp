/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
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


#include <QDebug>
#include <QBuffer>
#include <QFile>
#include <QDomDocument>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include "core.h"
#include "twitpicengine.h"


TwitPicEngine::TwitPicEngine( Core *coreParent, QObject *parent ) :
    QObject( parent ),
    manager( new QNetworkAccessManager( this ) ),
    reply( 0 ),
    coreParent( coreParent )
{
  connect( manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(readReply(QNetworkReply*)) );
  connect( this, SIGNAL(errorMessage(QString)), coreParent, SIGNAL(errorMessage(QString)) );
  connect( this, SIGNAL(completed(bool, QString, bool)), coreParent, SLOT(twitPicResponse(bool, QString, bool)) );
}

TwitPicEngine::~TwitPicEngine()
{
  if ( reply ) {
    reply->abort();
    reply->deleteLater();
    reply = 0;
  }
}

void TwitPicEngine::postContent( const QString &login, const QString &password, QString photoPath, QString status )
{ 
  QNetworkRequest request;

  QString url( "http://twitpic.com/api/upload" );
  if ( !status.isEmpty() ) {
    url.append( "AndPost" );
  }

  QFile photo( photoPath );
  photo.open( QIODevice::ReadOnly );

  QByteArray data;
  data.append( "--AaB03x\r\n" );
  data.append( "content-disposition: form-data; name=\"media\"; filename=\"" + photo.fileName().toAscii() + "\"\r\n" );
  data.append( "\r\n" );

  data.append( photo.readAll() );
  photo.close();

  data.append( "\r\n" );
  data.append( "--AaB03x\r\n" );
  if ( !status.isEmpty() ) {
    data.append( "content-disposition: form-data; name=\"message\"\r\n" );
    data.append( "\r\n" );
    data.append( status.toUtf8() + "\r\n" );
    data.append( "--AaB03x\r\n" );
  }
  data.append( "content-disposition: form-data; name=\"source\"\r\n" );
  data.append( "\r\n" );
  data.append( "qtwitter\r\n" );
  data.append( "--AaB03x\r\n" );
  data.append( "content-disposition: form-data; name=\"username\"\r\n" );
  data.append( "\r\n" );
  data.append( login.toUtf8() + "\r\n" );
  data.append( "--AaB03x\r\n" );
  data.append( "content-disposition: form-data; name=\"password\"\r\n" );
  data.append( "\r\n" );
  data.append( password.toUtf8() + "\r\n" );
  data.append( "--AaB03x--\r\n" );

  request.setHeader( QNetworkRequest::ContentTypeHeader, "multipart/form-data, boundary=AaB03x" );
  request.setUrl( QUrl( url ) );

  if ( reply ) {
    reply->abort();
    reply->deleteLater();
  }
  reply = manager->post( request, data );
  connect( reply, SIGNAL(uploadProgress(qint64,qint64)), coreParent, SIGNAL(twitPicDataSendProgress(qint64,qint64)) );
}

void TwitPicEngine::abort()
{
  qDebug() << "aborting...";
  reply->abort();
  reply->deleteLater();
  reply = 0;
}

void TwitPicEngine::readReply( QNetworkReply *reply )
{
  qDebug() << __PRETTY_FUNCTION__;
  int replyCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  if ( replyCode == 200 ) {
    parseReply( reply->readAll() );
  } else {
    emit errorMessage( "Download failed: " + reply->errorString() );
  }
  reply->close();
  reply = 0;
}


/*!
  Parses xml returned by Twitpic after the photo has been uploaded.

  \param &reply Contains the server's response.
*/
void TwitPicEngine::parseReply( const QByteArray &reply )
{
  QString url;
  bool userId = false;
  QDomDocument doc;

  doc.setContent(reply, false);
  QDomElement docElem = doc.documentElement();

  if ( docElem.hasAttribute("stat") && (docElem.attribute("stat") == "fail") ) {

    //failures have only att. "stat"
    QDomElement error = docElem.firstChild().toElement();
    qDebug() << error.tagName();
    if (!error.isNull()) {
      int errCode = error.attribute("code").toInt();
      QString errMsg;

      switch (errCode) {
      case ErrInvalidLogin:
        errMsg = tr( "Invalid twitter username or password");
        break;
      case ErrImageNotFound:
        errMsg = tr( "Image not found" );
        break;
      case ErrInvalidType:
        errMsg = tr( "Invalid image type" );
        break;
      case ErrOversized:
        errMsg = tr( "Image larger than 4MB" );
        break;
      default:
        errMsg = tr( "We're sorry, but due to some mysterious error your image failed to upload" );
      }
      emit completed(false, "\n" + errMsg, false);
      return;
    }
  }

  //status ok:
  QDomNode n = docElem.firstChild();
  while ( !n.isNull() ) {
    QDomElement e = n.toElement();
    if ( !e.isNull() ) {
      qDebug() << qPrintable( e.tagName() );
      if(e.tagName() == "userid")
        userId = true;
      else if(e.tagName() == "mediaurl")
        url = e.text();
    }
    n = n.nextSibling();
  }
  emit completed(true, url, userId);
}


/*! \class TwitPicEngine
    \brief A class responsible for interacting with TwitPic.

    This class provides an interface for communicating with TwitPic, for uploading
    and sharing photos.
*/

/*! \fn TwitPicEngine::TwitPicEngine( Core *coreParent, QObject *parent = 0 )
    Creates a new instance of TwitPicEngine class with the given \a coreParent and \a parent.
*/

/*! \fn TwitPicEngine::~TwitPicEngine();
    Destroys a TwitPicEngine instance.
*/

/*! \fn void TwitPicEngine::postContent( const QAuthenticator &authData, QString photoPath, QString status )
    This method constructs a post request based on an \a authData and \a status and
    appends the data of the image from the path given by \a photoPath. Issues a request
    when created.
    \param authData The authenticating user's login and password.
    \param photoPath A path to the image file on disk.
    \param status An optional status to be posted to Twitter with a link to the uploaded photo.
    \sa abort(), finished()
*/

/*! \fn void TwitPicEngine::abort()
    Aborts the current request.
*/

/*! \fn void TwitPicEngine::finished()
    Emitted for a finished request, with the content type specified as a parameter.
*/
