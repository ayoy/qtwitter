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


#include "httpconnection.h"

HttpConnection::HttpConnection( QObject *parent ) :
    QHttp( "/*url.host()*/", QHttp::ConnectionModeHttp, 80, parent ),
    bytearray( NULL ),
    buffer( NULL )
{
  connect( this, SIGNAL(requestStarted(int)), SLOT(httpRequestStarted(int)));
  connect( this, SIGNAL(requestFinished(int, bool)), SLOT(httpRequestFinished(int, bool)));
  connect( this, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), SLOT(readResponseHeader(const QHttpResponseHeader &)));
}

HttpConnection::~HttpConnection()
{
  if( buffer ) {
    delete buffer;
    buffer = NULL;
  }
  if( bytearray ) {
    delete bytearray;
    bytearray = NULL;
  }
}

void HttpConnection::setUrl( const QString &path )
{
  url.setUrl( path );
}

QByteArray HttpConnection::prepareRequest( const QString &path )
{
  url.setUrl( path );
  httpHostId = setHost( url.host(), QHttp::ConnectionModeHttp);
    
  bytearray = new QByteArray();
  buffer = new QBuffer( bytearray );

  if ( !buffer->open(QIODevice::ReadWrite) )
  {                                     
    emit errorMessage( tr("Unable to open device: ") + buffer->errorString() );
    delete buffer;
    buffer = 0;
    delete bytearray;
    bytearray = 0;
    return "invalid";
  }
  
  httpRequestAborted = false;
  QByteArray encodedPath = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
  if ( encodedPath.isEmpty() )
    encodedPath = "/";
  qDebug() << "About to download: " + encodedPath + " from: " + url.host();
  return encodedPath;
}

void HttpConnection::httpRequestStarted( int /*requestId*/ )
{
  //qDebug() << currentRequest().toString();
}
