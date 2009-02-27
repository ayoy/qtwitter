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

#include <QAuthenticator>
#include "httpconnection.h"
#include "xmlparser.h"
#include "xmlparserdirectmsg.h"

struct XmlData
{
  int id;
  QBuffer *buffer;
  QByteArray *bytearray;
  XmlData();
  ~XmlData();
  void assign( int newId, QBuffer *newBuffer, QByteArray *newByteArray );
  void clear();
};

class Core;

class XmlDownload : public HttpConnection
{
  Q_OBJECT

public:
  enum Role {
    RefreshAll,
    RefreshStatuses,
    Submit,
    Destroy
  };
  enum ContentRequested {
    Statuses,
    DirectMessages
  };

  XmlDownload( Role role, Core *coreParent, QObject *parent = 0 );
  void getContent( const QString &path, ContentRequested content );
  void postContent( const QString &path, const QByteArray &status, ContentRequested content );
  Role getRole() const;

signals:
  void finished( XmlDownload::ContentRequested );
  void xmlParsed();
  void cookieReceived( const QStringList );
  void deleteEntry( int id );
  void errorMessage( const QString& );

private slots:
  void extractId( Entry* );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestFinished( int requestId, bool error );

private:
  void createConnections( Core *whereToConnectTo );
  XmlData* processedRequest( ContentRequested content );
  XmlData* processedRequest( int requestId );
  Role role;
  XmlData statusesData;
  XmlData directMessagesData;
  XmlParser *statusParser;
  XmlParserDirectMsg *directMsgParser;
  Core *core;
  bool authenticating;
  bool authenticated;
};

#endif //XMLDOWNLOAD_H
