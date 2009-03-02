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

ImageData::ImageData() :
    image(0),
    bytearray(0),
    buffer(0)
{}

ImageData::~ImageData()
{
  if ( image ) {
    delete image;
    image = 0;
  }
  if ( bytearray ) {
    delete bytearray;
    bytearray = 0;
  }
  if ( buffer ) {
    delete buffer;
    buffer = 0;
  }
}

ImageDownload::ImageDownload() : HttpConnection() {}

ImageDownload::~ImageDownload()
{}

void ImageDownload::imageGet( Entry *entry )      //requestByEntry[entry.getId()] = httpGetId;
{
  QString imagePath = entry->image();
  QByteArray encodedPath = prepareRequest( imagePath );
  httpGetId = get( encodedPath, buffer );
  requestByEntry.insert( imagePath, httpGetId );
  ImageData *imageData = new ImageData;
  imageData->buffer = buffer;
  imageData->bytearray = bytearray;
  buffer = 0;
  bytearray = 0;
  imageByEntry.insert( imagePath, *imageData );
  qDebug() << "Request of type GET and id" << httpGetId << "started";
}

void ImageDownload::clearData()
{
  imageByEntry.clear();
  requestByEntry.clear();
}

void ImageDownload::httpRequestFinished( int requestId, bool error )
{
//  qDebug() << "finished" << state();
  if ( !requestByEntry.values().contains( requestId ) || httpRequestAborted ) {
    httpRequestAborted = false;
    return;
  }

  ImageData *imageData = &imageByEntry[ requestByEntry.key( requestId ) ];

  imageData->buffer->close();
  if (error) {
    emit errorMessage( tr("Download failed: ") + errorString() );
  }
  qDebug() << "Image request of id" << requestId << "finished" << requestByEntry.key( requestId );
  imageData->image = new QImage;
  imageData->image->loadFromData( *imageData->bytearray );
  emit imageReadyForUrl( requestByEntry.key( requestId ), *imageData->image );
}

void ImageDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
//  qDebug() << "Response for" << requestByEntry.key( currentId() );//url.path();
  qDebug() << "Code:" << responseHeader.statusCode() << ", status:" << responseHeader.reasonPhrase();
  switch ( responseHeader.statusCode() ) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  default:
    //emit errorMessage( tr( "Download failed: " ) + responseHeader.reasonPhrase() );
    httpRequestAborted = true;

//    imageByEntry.remove( requestByEntry.key( currentId() ) );
//    qDebug() << "BLABLABLA:" << requestByEntry.key( currentId() );

//    ImageData *imageData = &imageByEntry[ requestByEntry.key( currentId() ) ];
//    if ( imageData ) {
//      if ( imageData->buffer ) {
//        imageData->buffer->close();
//        delete imageData->buffer;
//        imageData->buffer = 0;
//      }
//      if ( imageData->bytearray ) {
//        delete imageData->bytearray;
//        imageData->bytearray = 0;
//      }
//    }
  }
}
