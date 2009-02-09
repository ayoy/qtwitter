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

#include "ui_authdialog.h"
#include "entry.h"

#include <QtNetwork>

class HttpConnection : public QHttp {

  Q_OBJECT

public:
  HttpConnection( QObject *parent = 0 );
  virtual ~HttpConnection();
  virtual void syncGet( const QString &path, bool isSync = false, QStringList cookie = QStringList() );
  void syncPost( const QString &path, const QByteArray &status, bool isSync = false, QStringList cookie = QStringList() );
  inline void setUrl( const QString &path );
  
public slots:
  virtual void httpRequestFinished( int requestId, bool error ) = 0;
  virtual void readResponseHeader( const QHttpResponseHeader &responseHeader ) = 0;
  virtual void httpRequestStarted( int requestId );

signals:
  void dataParsed( const QString& );
  void newEntry( const Entry&, int );
  void errorMessage( const QString& );

protected:
  QByteArray prepareRequest( const QString &path );
  QByteArray *bytearray;
  QBuffer *buffer;
  QUrl url;
  bool httpRequestAborted;
  int httpGetId;
  int httpHostId;
  int httpUserId;
  int closeId;
};

#endif //HTTPCONNECTION_H
