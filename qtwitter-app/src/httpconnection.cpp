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
    emit errorMessage( "Error: Unable to open device " + buffer->errorString() );
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

void HttpConnection::clearDataStorage()
{
  if (buffer) {
    if ( buffer->isOpen() ) {
      buffer->close();
    }
    delete buffer;
    buffer = 0;
  }
  if(bytearray) {
    delete bytearray;
    bytearray = 0;
  }
}

/*! \class HttpConnection
    \brief A base class for creating HTTP connection.

    This class provides an interface for creating and managing a connection
    over HTTP protocol. It inherits QHttp and includes handles for data
    retrieved by requests. By reimplementing its private slots a custom
    handling of the request's data can be provided.
*/

/*! \fn HttpConnection::HttpConnection( QObject *parent = 0 )
    The default constructor. Constructs an uninitialized HttpConnection instance
    with the given \a parent.
*/

/*! \fn virtual HttpConnection::~HttpConnection()
    A virtual destructor.
*/

/*! \fn inline void HttpConnection::setUrl( const QString &path )
    Sets the full URL for the object to be retrieved.
    \param path The given URL.
*/

/*! \fn void HttpConnection::errorMessage( const QString &message )
    Emitted to inform user about encountered problems.
    \param message Error message.
*/

/*! \fn QByteArray HttpConnection::prepareRequest( const QString &path )
    Sets the connection up by defining host, allocating memory for storing
    retrieved data, and converting \a path to a percent-encoded URL.
    \param path URL for the request.
    \sa bytearray, buffer
    \returns Percent-encoded URL prepared for passing to QHttp::get() or QHttp::post() method.
*/

/*! \fn void HttpConnection::clearDataStorage()
    Frees the memory allocated for the reply data.
*/

/*! \var bool HttpConnection::httpRequestAborted
    Is set to true when request is aborted.
*/

/*! \var int HttpConnection::httpGetId
    Stores the GET request id.
*/

/*! \var int HttpConnection::httpHostId
    Stores the request id returned by QHttp::setHost().
*/

/*! \var int HttpConnection::httpUserId
    Stores the request id returned by QHttp::setUser().
*/

/*! \var int HttpConnection::closeId
    Stores the request id returned by QHttp::close().
*/

/*! \var QUrl HttpConnection::url
    Stores the URL for the request.
*/

/*! \var QByteArray* HttpConnection::bytearray
    Provides the handle to where the downloaded data is stored.
    \sa buffer
*/

/*! \var QBuffer* HttpConnection::buffer
    Provides a convenient interface for access to \ref bytearray.
    \sa bytearray
*/
