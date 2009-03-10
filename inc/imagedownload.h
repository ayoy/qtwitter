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


#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include "httpconnection.h"

/*!
  \brief A struct containing data handles for retrieved images.

  Before creating a connection for image downlaod, an ImageData structure
  instance is created, and its \ref buffer and \ref bytearray members are
  assigned to corresponding ImageDownload class's members, that can be
  freed then. When download finishes, a downloaded image is assigned to
  \ref image field. All the memory management for ImageData struct is
  covered inside the ImageDownload class.
*/
struct ImageData
{
  QImage *image; /*!< A pointer to the image. */
  QByteArray *bytearray; /*!< A pointer to the bytearray. */
  QBuffer *buffer; /*!< A pointer to the buffer. */
  ImageData();
  ~ImageData();
};

/*!
  \brief A class for downlading images for Tweets.

  This is a class that provides interface for downloading user profile images
  for Tweets.
*/
class ImageDownload : public HttpConnection
{
  Q_OBJECT

public:
  ImageDownload(); /*!< A default constructor. */
  ~ImageDownload(); /*!< A destructor. */

  /*!
    This method invokes HttpConnection::prepareRequest() for a path
    given in \a entry, assigns allocated data to newly created
    ImageData instance and then issues QHttp::get().
    \param entry Entry for which the image will be downloaded.
  */
  void imageGet( Entry *entry );

  /*!
    Forces clearing of temporary images store, gathered while updating timeline.
  */
  void clearData();

signals:
  /*!
    Emitted when the request is finished and the image is ready to be displayed.
    \param path The image's URL.
    \param image The downloaded image.
  */
  void imageReadyForUrl( const QString& path, QImage image );

private slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );

private:
  QMap<QString,int> requestByEntry;
  QMap<QString,ImageData> imageByEntry;
};

#endif //IMAGEDOWNLOAD_H
