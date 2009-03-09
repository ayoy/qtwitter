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


#include <QPixmap>
#include <QAuthenticator>
#include "twitpicengine.h"

TwitPicEngine::TwitPicEngine( Core *coreParent, QObject *parent ) :
    HttpConnection( parent ),
    core( coreParent )
{
  replyParser = new TwitPicXmlParser( this );
  createConnections( core );
}

TwitPicEngine::~TwitPicEngine()
{
  if ( replyParser )
    replyParser->deleteLater();
}

void TwitPicEngine::postContent( const QAuthenticator &authData, QString photoPath, QString status )
{
  QFile photo( photoPath );
  photo.open( QIODevice::ReadOnly );
//  QByteArray bytes;
//  QBuffer imageBuffer( &bytes );
//  imageBuffer.open( QIODevice::WriteOnly );
//  pixmap.save( &imageBuffer, "jpeg" );
  //imageBuffer.close();

  QString path;
  QByteArray requestString;
  requestString.append( "media=" );
  requestString.append( photo.readAll() );
  requestString.append( "&username=" + authData.user() + "&password=" + authData.password() );
  photo.close();

  qDebug() << requestString;

  if ( status.isEmpty() ) {
    path = "http://twitpic.com/api/upload";
  } else {
    path = "http://twitpic.com/api/uploadAndPost";
    requestString.append( "&message=" + status.toAscii() );
  }
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }

  QHttpRequestHeader header( "POST", encodedPath );
  header.setContentType( "multipart/form-data" );
  qDebug() << header.toString() << header.isValid();
  httpGetId = request( header, requestString, buffer );
//  httpGetId = post( encodedPath, requestString, buffer );
  qDebug() << "Request of type POST and id" << httpGetId << "started";
  qDebug() << this->currentRequest().toString();// .contentType();
}

void TwitPicEngine::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  //qDebug() << responseHeader.values() ;// allValues( "Set-Cookie" );
  //emit cookieReceived( responseHeader.allValues( "Set-Cookie" ) );
  switch (responseHeader.statusCode()) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  default:
    qDebug() << "Download failed: " << responseHeader.reasonPhrase();
    //emit errorMessage( "Download failed: " + responseHeader.reasonPhrase() );
    httpRequestAborted = true;
    abort();
    clearDataStorage();
  }
}

void TwitPicEngine::httpRequestFinished(int requestId, bool error)
{
  closeId = close();
  if (httpRequestAborted) {
    clearDataStorage();
    qDebug() << "request aborted";
    return;
  }
  if (requestId != httpGetId )
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
//  emit finished();
}

void TwitPicEngine::createConnections( Core *coreParent )
{
//  connect( this, SIGNAL(finished(TwitPicEngine::ContentRequested)), coreParent, SLOT(setFlag(TwitPicEngine::ContentRequested)) );
  connect( this, SIGNAL(errorMessage(QString)), coreParent, SIGNAL(errorMessage(QString)) );
  connect( replyParser, SIGNAL(completed(bool,QString,bool)), coreParent, SLOT(twitPicResponse(bool,QString,bool)) );
}
