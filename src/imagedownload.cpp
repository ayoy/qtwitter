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


#include "imagedownload.h"

ImageDownload::ImageDownload() : HttpConnection(), userImage( NULL ) {}

ImageDownload::~ImageDownload()
{
  if ( userImage ) {
    delete userImage;
    userImage = NULL;
  }
}

void ImageDownload::syncGet( const QString &path, bool isSync )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  if ( userImage ) {
    delete userImage;
    userImage = NULL;
  }
  httpGetId = get( encodedPath, buffer );
  if ( isSync ) {
    qDebug() << "entering event loop...";
    getEventLoop.exec( QEventLoop::ExcludeUserInputEvents );
  }
  qDebug() << httpGetId;
}

void ImageDownload::httpRequestStarted( int requestId ) {
  if ( requestId == closeId ) {
    qDebug() << "close()" << state();
    if ( !state() ) {
      getEventLoop.quit();
    }
  }
}

void ImageDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  qDebug() << "Response for" << url.path();
  qDebug() << "Code is:" << responseHeader.statusCode() << ", status is:" << responseHeader.reasonPhrase() << "\n";
  switch ( responseHeader.statusCode() ) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  case 404:                   // Not Found
    if ( userImage ) {
      delete userImage;
      userImage = NULL;
    }
    userImage = new QImage( ":/icons/noimage.png" );
    emit imageDownloaded( url.toString(), *userImage );
    break;


  default:
    emit errorMessage( tr( "Download failed: " ) + responseHeader.reasonPhrase() );
    httpRequestAborted = true;
    abort();
    if ( buffer ) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if ( bytearray ) {
      delete bytearray;
      bytearray = 0;
    }
  }
}

void ImageDownload::httpRequestFinished( int requestId, bool error )
{
  if (httpRequestAborted) {
    if (buffer) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if(bytearray) {
      delete bytearray;
      bytearray = 0;
    }
    return;
  }
  if ( requestId == closeId ) {
    getEventLoop.quit();
    return;
  }
  if ( requestId != httpGetId )
    return;

  buffer->close();
  if (error) {
    emit errorMessage( tr("Download failed: ") + errorString() );
  }
  userImage = new QImage;
  qDebug() << url.toString().right(3);
  userImage->loadFromData( *bytearray );

  delete buffer;
  buffer = 0;
  delete bytearray;
  bytearray = 0;
  if ( state() != QHttp::Unconnected ) {
    closeId = close();
  } else {
    getEventLoop.quit();
  }
}

QImage ImageDownload::getUserImage()
{
  return *userImage;
}
