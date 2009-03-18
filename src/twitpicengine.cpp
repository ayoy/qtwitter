/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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
  QString path;
  if ( status.isEmpty() ) {
    path = "http://twitpic.com/api/upload";
  } else {
    path = "http://twitpic.com/api/uploadAndPost";
  }

  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
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
    requestString.append( status.toAscii() + "\r\n" );
    requestString.append( "--AaB03x\r\n" );
  }
  requestString.append( "content-disposition: form-data; name=\"source\"\r\n" );
  requestString.append( "\r\n" );
  requestString.append( "qTwitter\r\n" );
  requestString.append( "--AaB03x\r\n" );
  requestString.append( "content-disposition: form-data; name=\"username\"\r\n" );
  requestString.append( "\r\n" );
  requestString.append( authData.user().toAscii() + "\r\n" );
  requestString.append( "--AaB03x\r\n" );
  requestString.append( "content-disposition: form-data; name=\"password\"\r\n" );
  requestString.append( "\r\n" );
  requestString.append( authData.password().toAscii() + "\r\n" );
  requestString.append( "--AaB03x--\r\n" );


  QHttpRequestHeader header( "POST", encodedPath );
  header.setValue( "Host", "twitpic.com" );
  header.setValue( "Content-type", "multipart/form-data, boundary=AaB03x" );
  header.setValue( "Cache-Control", "no-cache" );
  header.setValue( "Accept","*/*" );
  header.setContentLength( requestString.length() );

  qDebug() << header.toString() << header.isValid();
  httpGetId = request( header, requestString, buffer );
  qDebug() << "Request of type POST and id" << httpGetId << "started";
  qDebug() << this->currentRequest().toString();
}

void TwitPicEngine::abort()
{
  qDebug() << "aborting...";
  httpRequestAborted = true;
  QHttp::abort();
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
}

void TwitPicEngine::createConnections( Core *coreParent )
{
  connect( this, SIGNAL(errorMessage(QString)), coreParent, SIGNAL(errorMessage(QString)) );
  connect( replyParser, SIGNAL(completed(bool,QString,bool)), coreParent, SLOT(twitPicResponse(bool,QString,bool)) );
  connect( this, SIGNAL(dataSendProgress(int,int)), coreParent, SIGNAL(twitPicDataSendProgress(int,int)) );
}
