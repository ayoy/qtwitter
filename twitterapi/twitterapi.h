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

#ifdef HAVE_OAUTH
#   include <QtOAuth>
#endif

class QNetworkReply;
class QAuthenticator;
class QXmlSimpleReader;
class QXmlInputSource;
class XmlParser;
struct Interface;

typedef QList<Entry> EntryList;

class TwitterAPIPrivate;

class TWITTERAPI_EXPORT TwitterAPI : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString login READ login WRITE setLogin );
    Q_PROPERTY( QString password READ password WRITE setPassword );
    Q_PROPERTY( QString serviceUrl READ serviceUrl WRITE setServiceUrl );
#ifdef HAVE_OAUTH
    Q_PROPERTY( bool usingOAuth READ isUsingOAuth WRITE setUsingOAuth );
    Q_PROPERTY( QByteArray consumerKey READ consumerKey WRITE setConsumerKey );
    Q_PROPERTY( QByteArray consumerSecret READ consumerSecret WRITE setConsumerSecret );
#endif

public:
    enum SocialNetwork {
        SOCIALNETWORK_TWITTER,
        SOCIALNETWORK_IDENTICA
    };

    /*!
      Set for sent requests, represents the activity performed by the specific request.
    */
    enum Role {
        ROLE_PUBLIC_TIMELINE = 101,
        ROLE_FRIENDS_TIMELINE,
        ROLE_MENTIONS,
        ROLE_DIRECT_MESSAGES,
        ROLE_POST_UPDATE,
        ROLE_DELETE_UPDATE,
        ROLE_POST_DM,
        ROLE_DELETE_DM,
        ROLE_FAVORITES_CREATE,
        ROLE_FAVORITES_DESTROY,
        ROLE_FRIENDSHIP_CREATE,
        ROLE_FRIENDSHIP_DESTROY
    };

    enum ErrorCode {
        ERROR_NO_ERROR = 0,
        ERROR_DM_NOT_ALLOWED,
        ERROR_DM_USER_NOT_FOUND
    };

    /*!
      A constant used as a "login" for public timeline requests.
    */
    static const QString PUBLIC_TIMELINE;

    static const QString URL_IDENTICA;
    static const QString URL_TWITTER;

    TwitterAPI( QObject *parent = 0 );
#ifdef HAVE_OAUTH
    TwitterAPI( const QString &serviceUrl, const QString &login, const QString &password,
                bool usingOAuth, QObject *parent = 0 );
#else
    TwitterAPI( const QString &serviceUrl, const QString &login, const QString &password,
                QObject *parent = 0 );
#endif
    virtual ~TwitterAPI();

    QString login() const;
    void setLogin( const QString &login );
    QString password() const;
    void setPassword( const QString &password );
    QString serviceUrl() const;
    void setServiceUrl( const QString &serviceUrl );
#ifdef HAVE_OAUTH
    bool isUsingOAuth() const;
    void setUsingOAuth( bool usingOAuth );
    QByteArray consumerKey() const;
    void setConsumerKey( const QByteArray &consumerKey );
    QByteArray consumerSecret() const;
    void setConsumerSecret( const QByteArray &consumerSecret );
#endif
    void postUpdate( const QString &data, quint64 inReplyTo = 0 );
    void deleteUpdate( quint64 id );
    void getTimelineRequest( QNetworkRequest &request, const QString &urlStatuses, Role role, int msgCount );
    void friendsTimeline( int msgCount = 20 );
    void mentions( int msgCount = 20 );
    void directMessages( int msgCount = 20 );
    void postDM( const QString &screenName, const QString &text );
    void deleteDM( quint64 id );
    void createFavorite( quint64 id );
    void destroyFavorite( quint64 id );
    void publicTimeline();

    void follow( quint64 userId );
    //  void follow( const QString &userLogin );
    void unfollow( quint64 userId );
    //  void unfollow( const QString &userLogin );

public slots:
    void resetConnections();

signals:
    void requestDone( int role );
    void newEntries( const EntryList &entries );
    void deleteEntry( quint64 id );
    void favoriteStatus( quint64 id, bool favorited );
    void postDMDone( TwitterAPI::ErrorCode error );
    void deleteDMDone( quint64 id, TwitterAPI::ErrorCode error );
    void followed( quint64 userId );
    void unfollowed( quint64 userId );

    void unauthorized();
    void unauthorized( const QString &status, quint64 inReplyToId );
    void unauthorized( const QString &screenName, const QString &text );
    void unauthorized( quint64 destroyId, Entry::Type type );
    void errorMessage( const QString &message );

private:
    TwitterAPIPrivate * const d_ptr;

    Q_DISABLE_COPY(TwitterAPI);
    Q_DECLARE_PRIVATE(TwitterAPI);

};

#endif // TWITTERAPI_H
