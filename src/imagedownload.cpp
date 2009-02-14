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

ImageDownload::ImageDownload() : HttpConnection() {}

ImageDownload::~ImageDownload()
{
//  imageByEntry.clear();
}

void ImageDownload::imgGet( const Entry &entry )      //requestByEntry[entry.getId()] = httpGetId;
{
  QString imgPath = entry.image();
  QByteArray encodedPath = prepareRequest( imgPath );
  qDebug() << "getting" << encodedPath << "\nentry:" << imgPath;
  httpGetId = get( encodedPath, buffer );
  qDebug() << httpGetId;
  requestByEntry.insert( imgPath, httpGetId );
  ImageData *imgData = new ImageData;
  imgData->buffer = buffer;
  imgData->bytearray = bytearray;
  imageByEntry.insert( imgPath, *imgData );
  qDebug() << "Request of type GET and id" << httpGetId << "started";
}

void ImageDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  qDebug() << "Response for" << requestByEntry.key( currentId() );//url.path();
  qDebug() << "Code is:" << responseHeader.statusCode() << ", status is:" << responseHeader.reasonPhrase() << "\n";
  switch ( responseHeader.statusCode() ) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  default:
    emit errorMessage( tr( "Download failed: " ) + responseHeader.reasonPhrase() );
    httpRequestAborted = true;
    abort();

    ImageData *imageData = &imageByEntry[ requestByEntry.key( currentId() ) ];
    if ( imageData->buffer ) {
      imageData->buffer->close();
      delete imageData->buffer;
      imageData->buffer = 0;
    }
    if ( imageData->bytearray ) {
      delete imageData->bytearray;
      imageData->bytearray = 0;
    }
  }
}

void ImageDownload::httpRequestFinished( int requestId, bool error )
{
  qDebug() << "finished";
  ImageData *imageData = &imageByEntry[ requestByEntry.key( requestId ) ];
  if (httpRequestAborted) {
    if (imageData->buffer) {
      imageData->buffer->close();
      delete imageData->buffer;
      imageData->buffer = 0;
    }
    if(imageData->bytearray) {
      delete imageData->bytearray;
      imageData->bytearray = 0;
    }
    return;
  }
  if ( !requestByEntry.values().contains( requestId ) )
    return;

  imageData->buffer->close();
  if (error) {
    emit errorMessage( tr("Download failed: ") + errorString() );
  }
  qDebug() << url.toString().right(3);
  qDebug() << "Request of id" << requestId << "finished";
  imageData->img = new QImage;
  imageData->img->loadFromData( *imageData->bytearray );
  emit imageReadyForUrl( requestByEntry.key( requestId ), *imageData->img );
//  delete imageData;
//  delete imageData->buffer;
//  imageData->buffer = 0;
//  delete imageData->bytearray;
//  imageData->bytearray = 0;
}
