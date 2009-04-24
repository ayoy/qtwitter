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


#include <QMap>
#include <QImage>
#include "imagedownload.h"

ImageData::ImageData() :
    image(0),
    bytearray(0),
    buffer(0)
{}

void ImageData::free()
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
    buffer->deleteLater();
    buffer = 0;
  }
}

ImageDownload::ImageDownload() : HttpConnection() {}

ImageDownload::~ImageDownload()
{
  QMap<QString,ImageData>::iterator i = imageByEntry.begin();
  while ( i != imageByEntry.end() ) {
    (*i).free();
    i++;
  }
}

void ImageDownload::imageGet( const QString &imageUrl )      //requestByEntry[entry.getId()] = httpGetId;
{
  QByteArray encodedPath = prepareRequest( imageUrl );
  httpGetId = get( encodedPath, buffer );
  requestByEntry.insert( imageUrl, httpGetId );
  ImageData *imageData = new ImageData;
  imageData->buffer = buffer;
  imageData->bytearray = bytearray;
  buffer = 0;
  bytearray = 0;
  imageByEntry.insert( imageUrl, *imageData );
//  qDebug() << "Request of type GET and id" << httpGetId << "started";
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
    emit errorMessage( tr("Download failed:").append( " " ) + errorString() );
  }
  qDebug() << "Image request of id" << requestId << "finished" << requestByEntry.key( requestId );
  imageData->image = new QImage;
  imageData->image->loadFromData( *imageData->bytearray );
//  if (!imageData->image->loadFromData( *imageData->bytearray ) ) {
//    qDebug() << "fail";
//  }
  emit imageReadyForUrl( requestByEntry.key( requestId ), *(imageData->image) );
  imageData->free();
  imageByEntry.remove( requestByEntry.key( requestId ) );
  requestByEntry.remove( requestByEntry.key( requestId ) );
}

void ImageDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
//  qDebug() << "Response for" << requestByEntry.key( currentId() );//url.path();
//  qDebug() << "Code:" << responseHeader.statusCode() << ", status:" << responseHeader.reasonPhrase();
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
  }
}

/*! \struct ImageData
    \brief A struct containing data handles for retrieved images.

    Before creating a connection for image downlaod, an ImageData structure
    instance is created, and its \ref buffer and \ref bytearray members are
    assigned to corresponding ImageDownload class's members, that can be
    freed then. When download finishes, a downloaded image is assigned to
    \ref image field. All the memory management for ImageData struct is
    covered inside the ImageDownload class.
*/

/*! \var QImage* ImageData::image
    A pointer to the image.
*/

/*! \var QByteArray* ImageData::bytearray
    A pointer to the bytearray.
*/

/*! \var QBuffer* ImageData::buffer
    A pointer to the buffer.
*/

/*! \fn ImageData::ImageData()
    Creates a new ImageData object with null pointers.
*/

/*! \fn ImageData::~ImageData()
    Destroys an instance of ImageData, freeing the memory allocated by its members.
*/

/*! \class ImageDownload
    \brief A class for downlading images for Tweets.

    This is a class that provides interface for downloading user profile images
    for Tweets.
*/

/*! \fn ImageDownload::ImageDownload()
    A default constructor.
*/

/*! \fn ImageDownload::~ImageDownload()
    A destructor.
*/

/*! \fn void ImageDownload::imageGet( Entry *entry )
    This method invokes HttpConnection::prepareRequest() for a path
    given in \a entry, assigns allocated data to newly created
    ImageData instance and then issues QHttp::get().
    \param entry Entry for which the image will be downloaded.
*/

/*! \fn void ImageDownload::clearData()
    Forces clearing of temporary images store, gathered while updating timeline.
*/

/*! \fn void ImageDownload::imageReadyForUrl( const QString& path, QImage image )
    Emitted when the request is finished and the image is ready to be displayed.
    \param path The image's URL.
    \param image The downloaded image.
*/
