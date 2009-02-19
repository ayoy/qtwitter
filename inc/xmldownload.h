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
#include "xmlparserdirectmsg.h"
#include <QAuthenticator>

struct XmlData {
  int id;
  QBuffer *buffer;
  QByteArray *bytearray;
  XmlData() :
      id(-1),
      buffer(0),
      bytearray(0)
  {}
  ~XmlData() {
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

class Core;

class XmlDownload : public HttpConnection {
  Q_OBJECT

  XmlData statusesData;
  XmlData directMessagesData;

public:
  enum Role {
    RefreshAll,
    RefreshStatuses,
    RefreshDirectMessages,
    Submit,
    Destroy
  };
  enum ContentRequested {
    Statuses,
    DirectMessages
  };

  XmlDownload( Role role, Core *coreParent, QObject *parent = 0 );
  Role role() const;
  void syncGet( const QString &path, ContentRequested content );
  void syncPost( const QString &path, const QByteArray &status, ContentRequested content );


public slots:
  void unlock();
  void lock();

private slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void extractId( Entry* );

signals:
  void xmlParsed();
  void cookieReceived( const QStringList );
  void deleteEntry( int id );
  void canBeUnlocked();

private:
  Role connectionRole;
  void createConnections( Core *whereToConnectTo );
  XmlParser *statusParser;
  XmlParserDirectMsg *directMsgParser;
  Core *core;
  bool authenticating;
  bool authenticated;
};

#endif //XMLDOWNLOAD_H
