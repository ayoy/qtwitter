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
#include "twitterapi.h"
#include "httpconnection.h"

class XmlParser;
class XmlParserDirectMsg;

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

class XmlDownload : public HttpConnection
{
  Q_OBJECT

public:
  XmlDownload( TwitterAPI::Role role, const QString &username, const QString &password, QObject *parent = 0 );
  ~XmlDownload();

  void getContent( const QString &path, TwitterAPI::ContentRequested content );
  void postContent( const QString &path, const QByteArray &status, TwitterAPI::ContentRequested content );

  TwitterAPI::Role getRole() const;
  QByteArray getPostStatus();
  int getPostInReplyToId();
  int getDestroyId();

  void setPostStatus( const QByteArray &newPostStatus );
  void setPostInReplyToId( int newId );
  void setDestroyId( int newId );

signals:
  void finished( TwitterAPI::ContentRequested content );
  void unauthorized();
  void unauthorized( const QByteArray &status, int inReplyToId );
  void unauthorized( int destroyId );
  void newEntry( Entry *entry );
  void deleteEntry( int id );

private slots:
  void extractId( Entry* );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestFinished( int requestId, bool error );


private:
  void createConnections();
  XmlData* processedRequest( TwitterAPI::ContentRequested content );
  XmlData* processedRequest( int requestId );
  TwitterAPI::Role role;
  QByteArray postStatus;
  int postInReplyToId;
  int destroyId;
  XmlData statusesData;
  XmlData directMessagesData;
  XmlParser *statusParser;
  XmlParserDirectMsg *directMsgParser;
  QAuthenticator authData;
  bool authenticating;
  bool authenticated;
};

#endif //XMLDOWNLOAD_H
