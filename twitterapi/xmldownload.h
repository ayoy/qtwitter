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

#include <QObject>
#include <QMap>
#include <QNetworkRequest>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include "twitterapi.h"

class QNetworkAccessManager;
class QNetworkReply;
class XmlParser;
class XmlParserDirectMsg;

class XmlDownload : public QObject
{
  Q_OBJECT

public:
  XmlDownload( QObject *parent = 0 );
  virtual ~XmlDownload();

  void postUpdate( const QString &login, const QString &password, const QString &data, int inReplyTo = -1 );
  void deleteUpdate( const QString &login, const QString &password, int id );
  void friendsTimeline( const QString &login, const QString &password );
  void directMessages( const QString &login, const QString &password );
  void publicTimeline();

signals:
  void finished( TwitterAPI::ContentRequested content );
  void errorMessage( const QString &message );
  void unauthorized();
  void unauthorized( const QString &status, int inReplyToId );
  void unauthorized( int destroyId );
  void newEntry( Entry *entry );
  void deleteEntry( int id );

private slots:
  void requestFinished( QNetworkReply *reply );
  void slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator );

private:
  void parseXml( const QByteArray &data, XmlParser *parser );
  QNetworkAccessManager* createNetworkAccessManager( const QString &login );
  QByteArray prepareRequest( const QString &data, int inReplyTo );
  QByteArray setAuthorizationData( const QString &login, const QString &password );
//  QMap<QString,QByteArray> statuses;
//  QMap<QString,QByteArray> dms;
  QMap<QString,QNetworkAccessManager*> connections;
  QMap<QString,bool> authStatuses;
  XmlParser *statusParser;
  XmlParserDirectMsg *directMsgParser;
  QXmlSimpleReader xmlReader;
  QXmlInputSource source;

  static const QNetworkRequest::Attribute ATTR_LOGIN;
  static const QNetworkRequest::Attribute ATTR_PASSWORD;
  static const QNetworkRequest::Attribute ATTR_STATUS;
  static const QNetworkRequest::Attribute ATTR_INREPLYTO_ID;
  static const QNetworkRequest::Attribute ATTR_DM_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_DELETION_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_DELETE_ID;
};

#endif //XMLDOWNLOAD_H
