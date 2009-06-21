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

#include "twitterapi_global.h"
#include "entry.h"

#include <QMap>
#include <QNetworkRequest>
#include <QPointer>

#ifdef OAUTH
#  include <QtOAuth>
#endif

class QNetworkReply;
class QAuthenticator;
class QXmlSimpleReader;
class QXmlInputSource;
class XmlParser;
struct Interface;

class TWITTERAPI_EXPORT TwitterAPIInterface : public QObject
{
  Q_OBJECT
#ifdef OAUTH
  Q_PROPERTY( QByteArray consumerKey READ consumerKey WRITE setConsumerKey )
  Q_PROPERTY( QByteArray consumerSecret READ consumerSecret WRITE setConsumerSecret )
#endif

public:

  TwitterAPIInterface( QObject *parent = 0 );
  virtual ~TwitterAPIInterface();

#ifdef OAUTH
  QByteArray consumerKey() const;
  void setConsumerKey( const QByteArray &consumerKey );
  QByteArray consumerSecret() const;
  void setConsumerSecret( const QByteArray &consumerSecret );
#endif

  void postUpdate( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &data, quint64 inReplyTo = 0 );
  void deleteUpdate( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id );
  void friendsTimeline( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, int msgCount = 20 );
  void directMessages( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, int msgCount = 20 );
  void postDM( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &screenName, const QString &text );
  void deleteDM( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id );
  void createFavorite( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id );
  void destroyFavorite( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id );
  void publicTimeline( TwitterAPI::SocialNetwork network );

public slots:
  void resetConnections();

signals:
  void requestDone( TwitterAPI::SocialNetwork network, const QString &login, int role );
  void newEntry( TwitterAPI::SocialNetwork network, const QString &login, Entry entry );
  void deleteEntry( TwitterAPI::SocialNetwork network, const QString &login, quint64 id );
  void favoriteStatus( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, bool favorited );
  void postDMDone( TwitterAPI::SocialNetwork network, const QString &login, TwitterAPI::ErrorCode error );
  void deleteDMDone( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, TwitterAPI::ErrorCode error );

  void unauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password );
  void unauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &status, quint64 inReplyToId );
  void unauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &screenName, const QString &text );
  void unauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 destroyId, Entry::Type type );
  void errorMessage( const QString &message );

private slots:
  void requestFinished( QNetworkReply *reply );
  void slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator );

private:
  void parseXml( const QByteArray &data, XmlParser *parser );
  Interface* createInterface( TwitterAPI::SocialNetwork network, const QString &login );

#ifdef OAUTH
  QByteArray prepareOAuthString( const QString &requestUrl, QOAuth::HttpMethod method,
                                 const QString &password, const QOAuth::ParamMap &params = QOAuth::ParamMap() );
#endif

  QByteArray prepareRequest( const QString &data, quint64 inReplyTo );
  QByteArray prepareRequest( const QString &screenName, const QString & );

  QMap< TwitterAPI::SocialNetwork, QMap<QString,Interface*> > connections;
  QMap< TwitterAPI::SocialNetwork, QString > services;
  QXmlSimpleReader *xmlReader;
  QXmlInputSource *source;

#ifdef OAUTH
  QOAuth *qoauth;
#endif

  static const QNetworkRequest::Attribute ATTR_SOCIALNETWORK;
  static const QNetworkRequest::Attribute ATTR_ROLE;
  static const QNetworkRequest::Attribute ATTR_LOGIN;
  static const QNetworkRequest::Attribute ATTR_PASSWORD;
  static const QNetworkRequest::Attribute ATTR_STATUS;
  static const QNetworkRequest::Attribute ATTR_STATUS_ID;
  static const QNetworkRequest::Attribute ATTR_DM_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_DM_RECIPIENT;
  static const QNetworkRequest::Attribute ATTR_DELETION_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_MSGCOUNT;

  static const QString UrlStatusesPublicTimeline;
  static const QString UrlStatusesFriendsTimeline;
  static const QString UrlStatusesUpdate;
  static const QString UrlStatusesDestroy;
  static const QString UrlDirectMessages;
  static const QString UrlDirectMessagesNew;
  static const QString UrlDirectMessagesDestroy;
  static const QString UrlFavoritesCreate;
  static const QString UrlFavoritesDestroy;

};

#endif // TWITTERAPI_H
