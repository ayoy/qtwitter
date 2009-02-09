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


#ifndef XMLDOWNLOAD_H
#define XMLDOWNLOAD_H

#include "httpconnection.h"
#include "xmlparser.h"
#include <QAuthenticator>

class Core;

class XmlDownload : public HttpConnection {
  Q_OBJECT

public:
  XmlDownload( QAuthenticator _authData, Core *coreParent, bool isForGet = false, QObject *parent = 0 );
  XmlDownload( QAuthenticator _authData, int type, Core *coreParent, bool isForGet = false, QObject *parent = 0 );

public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void setAuthData( const QAuthenticator );
signals:
  void xmlParsed();
  void cookieReceived( const QStringList );

private:
  void createConnections( Core *whereToConnectTo, bool isForGet = false );
  QAuthenticator authData;
  XmlParser parser;
  Core *core;
  bool authenticated;
};

#endif //XMLDOWNLOAD_H
