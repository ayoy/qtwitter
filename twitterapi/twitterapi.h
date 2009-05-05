/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
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


#ifndef TWITTERAPI_H
#define TWITTERAPI_H

#include <QObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QPointer>
#include "xmlparser.h"

class QNetworkReply;

struct Interface
{
  QPointer<QNetworkAccessManager> connection;
  QPointer<XmlParser> statusParser;
  QPointer<XmlParserDirectMsg> directMsgParser;
  bool authorized;
  bool friendsInProgress;
  bool dmScheduled;
  ~Interface() {
    if ( connection )
      connection.data()->deleteLater();
    if ( statusParser )
      statusParser.data()->deleteLater();
    if ( directMsgParser )
      directMsgParser.data()->deleteLater();
  }
};

class TWITTERAPI_EXPORT TwitterAPI : public QObject
{
  Q_OBJECT

public:
  static const QString PUBLIC_TIMELINE;

  enum Role {
    ROLE_PUBLIC_TIMELINE = 101,
    ROLE_FRIENDS_TIMELINE,
    ROLE_DIRECT_MESSAGES,
    ROLE_POST_UPDATE,
    ROLE_DELETE_UPDATE,
    ROLE_POST_DM,
    ROLE_DELETE_DM
  };

  TwitterAPI( QObject *parent = 0 );
  virtual ~TwitterAPI();

  void postUpdate( const QString &login, const QString &password, const QString &data, int inReplyTo = -1 );
  void deleteUpdate( const QString &login, const QString &password, int id );
  void friendsTimeline( const QString &login, const QString &password, int msgCount = 20 );
  void directMessages( const QString &login, const QString &password, int msgCount = 20 );
  void postDM( const QString &login, const QString &password, const QString &user, const QString &data );
  void deleteDM( const QString &login, const QString &password, int id );
  void publicTimeline();

public slots:
  void resetConnections();

signals:
  void requestDone( const QString &login, int role );
  void newEntry( const QString &login, Entry entry );
  void deleteEntry( const QString &login, int id );
  void errorMessage( const QString &message );
  void unauthorized( const QString &login, const QString &password );
  void unauthorized( const QString &login, const QString &password, const QString &status, int inReplyToId );
  void unauthorized( const QString &login, const QString &password, int destroyId );

private slots:
  void requestFinished( QNetworkReply *reply );
  void slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator );

private:
  void parseXml( const QByteArray &data, XmlParser *parser );
  Interface* createInterface( const QString &login );
  QByteArray prepareRequest( const QString &data, int inReplyTo );

  QMap<QString,Interface*> connections;
  QXmlSimpleReader xmlReader;
  QXmlInputSource source;

  static const QNetworkRequest::Attribute ATTR_ROLE;
  static const QNetworkRequest::Attribute ATTR_LOGIN;
  static const QNetworkRequest::Attribute ATTR_PASSWORD;
  static const QNetworkRequest::Attribute ATTR_STATUS;
  static const QNetworkRequest::Attribute ATTR_INREPLYTO_ID;
  static const QNetworkRequest::Attribute ATTR_DM_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_DELETION_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_DELETE_ID;
  static const QNetworkRequest::Attribute ATTR_MSGCOUNT;
};

#endif // TWITTERAPI_H
