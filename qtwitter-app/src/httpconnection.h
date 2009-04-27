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


#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QtNetwork>

class HttpConnection : public QHttp
{
  Q_OBJECT

public:
  HttpConnection( QObject *parent = 0 );
  virtual ~HttpConnection();

  inline void setUrl( const QString &path );

signals:
  void errorMessage( const QString &message );

protected:
  QByteArray prepareRequest( const QString &path );
  void clearDataStorage();
  bool httpRequestAborted;
  int httpGetId;
  int httpHostId;
  int httpUserId;
  int closeId;
  QUrl url;
  QByteArray *bytearray;
  QBuffer *buffer;

private slots:
  virtual void httpRequestFinished( int requestId, bool error ) = 0;
  virtual void readResponseHeader( const QHttpResponseHeader &responseHeader ) = 0;
  virtual void httpRequestStarted( int requestId );
};

#endif //HTTPCONNECTION_H
