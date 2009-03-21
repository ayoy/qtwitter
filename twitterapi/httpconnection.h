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


#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "entry.h"

#include <QtNetwork>

/*!
  \brief A base class for creating HTTP connection.

  This class provides an interface for creating and managing a connection
  over HTTP protocol. It inherits QHttp and includes handles for data
  retrieved by requests. By reimplementing its private slots a custom
  handling of the request's data can be provided.
*/
class HttpConnection : public QHttp
{
  Q_OBJECT

public:
  /*!
    The default constructor. Constructs an uninitialized HttpConnection instance
    with the given \a parent.
  */
  HttpConnection( QObject *parent = 0 );

  /*!
    A virtual destructor.
  */
  virtual ~HttpConnection();

  /*!
    Sets the full URL for the object to be retrieved.
    \param path The given URL.
  */
  inline void setUrl( const QString &path );

signals:
  /*!
    Emitted to inform user about encountered problems.
    \param message Error message.
  */
  void errorMessage( const QString &message );

protected:
  /*!
    Sets the connection up by defining host, allocating memory for storing
    retrieved data, and converting \a path to percent-encoded URL.
    \param path URL for the request.
    \sa bytearray, buffer
    \returns Percent-encoded URL prepared for passing to QHttp::get() or QHttp::post() method.
  */
  QByteArray prepareRequest( const QString &path );

  /*!
    Frees the memory allocated for the reply data.
  */
  void clearDataStorage();
  bool httpRequestAborted; /*!< Is set to true when request is aborted. */
  int httpGetId; /*!< Stores the GET request id. */
  int httpHostId; /*!< Stores the request id returned by QHttp::setHost(). */
  int httpUserId; /*!< Stores the request id returned by QHttp::setUser(). */
  int closeId; /*!< Stores the request id returned by QHttp::close(). */
  QUrl url; /*!< Stores the URL for the request */
  QByteArray *bytearray; /*!< Provides the handle to where the downloaded data is stored. \sa buffer */
  QBuffer *buffer; /*!< Provides a convenient interface for access to \ref bytearray. \sa bytearray */

private slots:
  virtual void httpRequestFinished( int requestId, bool error ) = 0;
  virtual void readResponseHeader( const QHttpResponseHeader &responseHeader ) = 0;
  virtual void httpRequestStarted( int requestId );
};

#endif //HTTPCONNECTION_H
