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


#include "twitpicengine.h"
#include "twitpicxmlparser.h"

TwitPicEngine::TwitPicEngine( Core *coreParent, QObject *parent ) :
    QHttp( parent )
{
  replyParser = new TwitPicXmlParser( this );
  setHost( "twitpic.com", QHttp::ConnectionModeHttp);
  createConnections( coreParent );
}

TwitPicEngine::~TwitPicEngine()
{
  clearDataStorage();
}

void TwitPicEngine::postContent( const QString &login, const QString &password, QString photoPath, QString status )
{
  QString path;
  if ( status.isEmpty() ) {
    path = "/api/upload";
  } else {
    path = "/api/uploadAndPost";
  }

  QFile photo( photoPath );
  photo.open( QIODevice::ReadOnly );

  QByteArray requestString;
  requestString.append( "--AaB03x\r\n" );
  requestString.append( "content-disposition: form-data; name=\"media\"; filename=\"" + photo.fileName().toAscii() + "\"\r\n" );
  requestString.append( "\r\n" );

  requestString.append( photo.readAll() );
  photo.close();

  requestString.append( "\r\n" );
  requestString.append( "--AaB03x\r\n" );
  if ( !status.isEmpty() ) {
    requestString.append( "content-disposition: form-data; name=\"message\"\r\n" );
    requestString.append( "\r\n" );
    requestString.append( status.toUtf8() + "\r\n" );
    requestString.append( "--AaB03x\r\n" );
  }
  requestString.append( "content-disposition: form-data; name=\"source\"\r\n" );
  requestString.append( "\r\n" );
  requestString.append( "qtwitter\r\n" );
  requestString.append( "--AaB03x\r\n" );
  requestString.append( "content-disposition: form-data; name=\"username\"\r\n" );
  requestString.append( "\r\n" );
  requestString.append( login.toUtf8() + "\r\n" );
  requestString.append( "--AaB03x\r\n" );
  requestString.append( "content-disposition: form-data; name=\"password\"\r\n" );
  requestString.append( "\r\n" );
  requestString.append( password.toUtf8() + "\r\n" );
  requestString.append( "--AaB03x--\r\n" );


  QHttpRequestHeader header( "POST", path );
  header.setValue( "Host", "twitpic.com" );
  header.setValue( "Content-type", "multipart/form-data, boundary=AaB03x" );
  header.setValue( "Cache-Control", "no-cache" );
  header.setValue( "Accept","*/*" );
  header.setContentLength( requestString.length() );

  qDebug() << header.toString() << header.isValid();

  bytearray = new QByteArray;
  buffer = new QBuffer( bytearray );
  httpRequestAborted = false;

  httpGetId = request( header, requestString, buffer );
  qDebug() << "Request of type POST and id" << httpGetId << "started";
  qDebug() << currentRequest().toString();
}

void TwitPicEngine::abort()
{
  qDebug() << "aborting...";
  httpRequestAborted = true;
  QHttp::abort();
}

void TwitPicEngine::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  if ( responseHeader.statusCode() >= 400 ) {
    qDebug() << "Download failed: " << responseHeader.reasonPhrase();
    abort();
    clearDataStorage();
  }
}

void TwitPicEngine::httpRequestFinished(int requestId, bool error)
{
  if (httpRequestAborted) {
    clearDataStorage();
    qDebug() << "request aborted";
    return;
  }
  if ( requestId != httpGetId )
    return;

  buffer->close();

  if (error) {
    emit errorMessage( "Download failed: " + errorString() );
  } else {
    QXmlSimpleReader xmlReader;
    QXmlInputSource source;
    source.setData( *bytearray );
    xmlReader.setContentHandler( replyParser );
    xmlReader.parse( source );
    qDebug() << "========= XML PARSING FINISHED =========" << state();
  }
  clearDataStorage();
}

void TwitPicEngine::createConnections( Core *coreParent )
{
  connect( this, SIGNAL(requestStarted(int)), SLOT(httpRequestStarted(int)));
  connect( this, SIGNAL(requestFinished(int, bool)), SLOT(httpRequestFinished(int, bool)));
  connect( this, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), SLOT(readResponseHeader(const QHttpResponseHeader &)));

  connect( this, SIGNAL(errorMessage(QString)), coreParent, SIGNAL(errorMessage(QString)) );
  connect( replyParser, SIGNAL(completed(bool,QString,bool)), coreParent, SLOT(twitPicResponse(bool,QString,bool)) );
  connect( this, SIGNAL(dataSendProgress(int,int)), coreParent, SIGNAL(twitPicDataSendProgress(int,int)) );
}

void TwitPicEngine::clearDataStorage()
{
  if (buffer) {
    if ( buffer->isOpen() ) {
      buffer->close();
    }
    delete buffer;
    buffer = 0;
  }
  if(bytearray) {
    delete bytearray;
    bytearray = 0;
  }
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
