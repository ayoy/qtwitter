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

struct ImageData {
  QImage *img;
  QByteArray *bytearray;
  QBuffer *buffer;
  ImageData() :
    img(0),
    bytearray(0),
    buffer(0)
    {}
  ~ImageData() {
    if ( img ) {
      delete img;
      img = 0;
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
};

class ImageDownload : public HttpConnection {
  Q_OBJECT
public:
  ImageDownload();
  ~ImageDownload();
  void imgGet( const Entry &entry );

private:
  QMap<QString,int> requestByEntry;
  QMap<QString,ImageData> imageByEntry;

private slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );

signals:  
  void imageDownloaded( const QString&, QImage );
  void imageReadyForUrl( const QString&, QImage );
};

#endif //IMAGEDOWNLOAD_H
