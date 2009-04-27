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


#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include <QImage>
#include <twitterapi/twitterapi.h>
#include "httpconnection.h"

struct ImageData
{
  QImage *image;
  QByteArray *bytearray;
  QBuffer *buffer;
  ImageData();
  void free();
};

class ImageDownload : public HttpConnection
{
  Q_OBJECT

public:
  ImageDownload();
  ~ImageDownload();

  void imageGet( const QString& imageUrl );
  void clearData();

signals:
  void imageReadyForUrl( const QString& path, QImage image );

private slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );

private:
  QMap<QString,int> requestByEntry;
  QMap<QString,ImageData> imageByEntry;
};

#endif //IMAGEDOWNLOAD_H
