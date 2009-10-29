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


#include "twitterapi.h"
#include "twitterapi_p.h"
#include "xmlparser.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QAuthenticator>
#include <QDebug>
#include <QThreadPool>

struct Interface
{
    QPointer<QNetworkAccessManager> connection;
    bool authorized;
    bool friendsInProgress;
    bool dmScheduled;
    ~Interface() {
        if ( connection )
            connection.data()->deleteLater();
    }
};


/*!
  \class TwitterAPIInterface
  \brief A class interacting with Twitter.

  This class provides an interface for connecting to Twitter REST API.
  Its usage boils down to instantiating it and issuing requests for retrieving
  needed data, like publicTimeline(), friendsTimeline() or directMessages().

  Once a timeline or direct messages GET or POST request completes successfully,
  the received XML document is parsed. The newEntry() signal is emitted with every
  new status or direct message being parsed. Requests for deleting statuses are
  also sent as POST requests, but the responses aren't parsed at all. Instead,
  the HTTP reply code is read. If it's equal to 200 (OK), the deleteEntry()
  signal is emitted. The current Twitter API implementation allows for it,
  since it replies with code other than 200 if status couldn't be deleted.

  Every newEntry() and deleteEntry() signal, apart from parsed Entry (or id of
  the status in case of deleteEntry()) contains also the login of the account
  for which a status is received or deleted, so that sender of the request
  could easily figure out what to do with the given Entry (id).

  For requests requiring authentication (namely - all but publicTimeline())
  a login and password have to be provided. If user cannot be authenticated
  with the given password, the appropriate unauthorized() signal is emitted,
  and the request is aborted.
*/

/*!
  \fn void TwitterAPI::requestDone( const QString &login, int role )

  Emitted when the request of a given \a role for a user identified
  by \a login is completed and its data is read. If the request
  completed successfully, the whole data is supposed to be parsed.
  If there was an error processing the request, a separate
  errorMessage() signal is supposed to be emitted just before this.

  \param login User's login.
  \param role Specifies the request's role.

  \sa errorMessage(), Role
*/

/*!
  \fn void TwitterAPI::newEntry( const QString &login, Entry entry )

  Emitted when a new status or direct message is parsed from a Twitter API's
  reply for a request for the user identified by \a login. Used for both
  GET- and POST-type requests.

  \param login User's login.
  \param entry The parsed entry.

  \sa publicTimeline(), friendsTimeline(), directMessages(), postUpdate()
*/

/*!
  \fn void TwitterAPI::deleteEntry( const QString &login, quint64 id )

  Emitted when a status of the given \a id was deleted from Twitter and
  can be deleted from locally stored list of the user specified by \a login.

  \param login User's login.
  \param id An id of the status to be deleted.

  \sa deleteUpdate()
*/

/*!
  \fn void TwitterAPI::errorMessage( const QString &message )

  Emitted every time when something goes wrong with processing a request.
  This includes connection errors as well as Twitter internal errors.

  \param message An error message.
*/

/*!
  \fn void TwitterAPI::unauthorized( const QString &login, const QString &password )

  Emitted when the given credentials are rejected by Twitter. Aborts the pending
  request immediately.

  Emitted for GET requests, i.e. friendsTimeline() and directMessages().

  \param login Login that was rejected.
  \param password Password that was rejected.

  \sa friendsTimeline(), directMessages()
*/

/*!
  \fn void TwitterAPI::unauthorized( const QString &login, const QString &password, const QString &status, quint64 inReplyToStatusId )

  Emitted when the given credentials are rejected by Twitter. Aborts the pending
  request immediately.

  Emitted for postUpdate() request, provides also details of the request,
  like \a status and \a inReplyToStatusId.

  \param login Login that was rejected.
  \param password Password that was rejected.
  \param status Status message that was requested to be sent.
  \param inReplyToStatusId Optional id of the message that the requested status is replying to.

  \sa postUpdate()
*/

/*!
  \fn void TwitterAPI::unauthorized( const QString &login, const QString &password, int destroyId )

  Emitted when the given credentials are rejected by Twitter. Aborts the pending
  request immediately.

  Emitted for deleteUpdate() request, provides also details of the request,
  namely \a destroyId.

  \param login Login that was rejected.
  \param password Password that was rejected.
  \param destroyId Id of the status that was requested to be deleted.

  \sa deleteUpdate()
*/

const QString TwitterAPI::PUBLIC_TIMELINE = "public timeline";

const QString TwitterAPI::URL_IDENTICA = "https://identi.ca/api";
const QString TwitterAPI::URL_TWITTER = "https://twitter.com";


const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_SOCIALNETWORK      = (QNetworkRequest::Attribute) QNetworkRequest::User;
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_ROLE               = (QNetworkRequest::Attribute) (QNetworkRequest::User + 1);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_LOGIN              = (QNetworkRequest::Attribute) (QNetworkRequest::User + 2);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_PASSWORD           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 3);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_STATUS             = (QNetworkRequest::Attribute) (QNetworkRequest::User + 4);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_ID                 = (QNetworkRequest::Attribute) (QNetworkRequest::User + 5);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_DM_REQUESTED       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 6);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_DM_RECIPIENT       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 7);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_DELETION_REQUESTED = (QNetworkRequest::Attribute) (QNetworkRequest::User + 8);
const QNetworkRequest::Attribute TwitterAPIPrivate::ATTR_MSGCOUNT           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 9);

const QString TwitterAPIPrivate::UrlStatusesPublicTimeline  = "/statuses/public_timeline.xml";
const QString TwitterAPIPrivate::UrlStatusesFriendsTimeline = "/statuses/friends_timeline.xml";
const QString TwitterAPIPrivate::UrlStatusesMentions        = "/statuses/mentions.xml";
const QString TwitterAPIPrivate::UrlStatusesUpdate          = "/statuses/update.xml";
const QString TwitterAPIPrivate::UrlStatusesDestroy         = "/statuses/destroy/%1.xml";
const QString TwitterAPIPrivate::UrlDirectMessages          = "/direct_messages.xml";
const QString TwitterAPIPrivate::UrlDirectMessagesNew       = "/direct_messages/new.xml";
const QString TwitterAPIPrivate::UrlDirectMessagesDestroy   = "/direct_messages/destroy/%1.xml";
const QString TwitterAPIPrivate::UrlFavoritesCreate         = "/favorites/create/%1.xml";
const QString TwitterAPIPrivate::UrlFavoritesDestroy        = "/favorites/destroy/%1.xml";
const QString TwitterAPIPrivate::UrlFriendshipCreate        = "/friendships/create/%1.xml";
const QString TwitterAPIPrivate::UrlFriendshipDestroy       = "/friendships/destroy/%1.xml";


TwitterAPIPrivate::~TwitterAPIPrivate()
{
    delete iface;
    iface = 0;
}

void TwitterAPIPrivate::init( const QString &m_serviceUrl, const QString &m_login,
                              const QString &m_password, bool m_usingOAuth )
{
    qRegisterMetaType<EntryList>( "EntryList" );

    login = m_login;
    password = m_password;
    serviceUrl = m_serviceUrl;
    usingOAuth = m_usingOAuth;

    createInterface();
#ifdef HAVE_OAUTH
    qoauth = new QOAuth::Interface( this );
#endif
}

void TwitterAPIPrivate::createInterface()
{
    iface = new Interface;
    iface->connection = new QNetworkAccessManager( this );

    iface->friendsInProgress = false;
    iface->authorized = false;
    iface->dmScheduled = false;

    if ( login != TwitterAPI::PUBLIC_TIMELINE ) {
        connect( iface->connection, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                 SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)) );
    }
    connect( iface->connection, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrors(QNetworkReply*,QList<QSslError>)) );
    connect( iface->connection, SIGNAL(finished(QNetworkReply*)), SLOT(requestFinished(QNetworkReply*)) );
}

void TwitterAPIPrivate::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors )
{
    Q_UNUSED(errors);

    // TODO:
    reply->ignoreSslErrors();
}

/*!
  Constructs a new instance with a given \a parent.
*/
TwitterAPI::TwitterAPI( QObject *parent ) :
        QObject( parent ),
        d_ptr( new TwitterAPIPrivate )
{
    Q_D(TwitterAPI);

    d->q_ptr = this;
    d->init( QString(), QString(), QString() );
}

#ifdef HAVE_OAUTH
TwitterAPI::TwitterAPI( const QString &serviceUrl, const QString &login,
                        const QString &password, bool usingOAuth, QObject *parent ) :
        QObject( parent ),
        d_ptr( new TwitterAPIPrivate )
{
    Q_D(TwitterAPI);

    d->q_ptr = this;

    d->init( serviceUrl, login, password, usingOAuth );
}
#else
TwitterAPI::TwitterAPI( const QString &serviceUrl, const QString &login,
                        const QString &password, QObject *parent ) :
        QObject( parent ),
        d_ptr( new TwitterAPIPrivate )
{
    Q_D(TwitterAPI);

    d->q_ptr = this;

    d->init( serviceUrl, login, password, false );
}
#endif

/*!
  A destructor.
*/
TwitterAPI::~TwitterAPI()
{
    delete d_ptr;
}

QString TwitterAPI::login() const
{
    Q_D(const TwitterAPI);

    return d->login;
}

void TwitterAPI::setLogin( const QString & login )
{
    Q_D(TwitterAPI);

    d->login = login;
}

QString TwitterAPI::password() const
{
    Q_D(const TwitterAPI);

    return d->password;
}

void TwitterAPI::setPassword( const QString &password )
{
    Q_D(TwitterAPI);

    d->password = password;
}

QString TwitterAPI::serviceUrl() const
{
    Q_D(const TwitterAPI);

    return d->serviceUrl;
}

void TwitterAPI::setServiceUrl( const QString &serviceUrl )
{
    Q_D(TwitterAPI);

    d->serviceUrl = serviceUrl;
}

#ifdef HAVE_OAUTH
bool TwitterAPI::isUsingOAuth() const
{
    Q_D(const TwitterAPI);

    return d->usingOAuth;
}

void TwitterAPI::setUsingOAuth( bool usingOAuth )
{
    Q_D(TwitterAPI);

    d->usingOAuth = usingOAuth;
}

QByteArray TwitterAPI::consumerKey() const
{
    Q_D(const TwitterAPI);

    return d->qoauth->consumerKey();
}

void TwitterAPI::setConsumerKey( const QByteArray &consumerKey )
{
    Q_D(TwitterAPI);

    d->qoauth->setConsumerKey( QByteArray(consumerKey) );
}

QByteArray TwitterAPI::consumerSecret() const
{
    Q_D(const TwitterAPI);

    return d->qoauth->consumerSecret();
}

void TwitterAPI::setConsumerSecret( const QByteArray &consumerSecret )
{
    Q_D(TwitterAPI);

    d->qoauth->setConsumerSecret( consumerSecret );
}

#endif


/*!
  Sends a request to post a status update for the user identified by \a login
  and \a password.

  \param login User's login.
  \param password User's pasword.
  \param data A status message to be posted.
  \param inReplyTo Optional argument specifying an id of the status to which
                   the currently posted status replies.

  \sa newEntry(), deleteUpdate()
*/
void TwitterAPI::postUpdate( const QString &data, quint64 inReplyTo )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlStatusesUpdate );

    QByteArray content;
    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        QOAuth::ParamMap map;

        map.insert( "status", data.toUtf8().toPercentEncoding() );
        map.insert( "source", "qtwitter" );
        if ( inReplyTo != 0 ) {
            map.insert( "in_reply_to_status_id", QByteArray::number( inReplyTo ) );
        }

        d->oauthForPost( request, url, map );
        content = d->qoauth->inlineParameters( map );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
        content = d->prepareRequest( data, inReplyTo );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_POST_UPDATE );
    request.setAttribute( TwitterAPIPrivate::ATTR_STATUS, data );
    request.setAttribute( TwitterAPIPrivate::ATTR_ID, inReplyTo );

    d->iface->connection.data()->post( request, content );
}

/*!
  Sends a request to delete a status of a given \a id for the user identified
  by \a login and \a password.

  \param login User's login.
  \param password User's password.
  \param id An id of the status to be deleted.

  \sa deleteEntry(), postUpdate()
*/
void TwitterAPI::deleteUpdate( quint64 id )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlStatusesDestroy.arg( QString::number(id) ) );

    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        d->oauthForPost( request, url );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_DELETE_UPDATE );
    request.setAttribute( TwitterAPIPrivate::ATTR_DELETION_REQUESTED, true );
    request.setAttribute( TwitterAPIPrivate::ATTR_ID, id );

    d->iface->connection.data()->post( request, QByteArray() );
}

/*!
  Returns a request for getting a timeline for a logged in user.
  Length of the timeline can be adjusted by \a msgCount.

  \param msgCount Argument specifying length of requested timeline.
                  Twitter API currently accepts values up to 200.

  \sa friendsTimeline(), mentionsTimeline()
*/
void TwitterAPI::getTimelineRequest( QNetworkRequest &request, const QString &urlStatuses, Role role, int msgCount )
{
    Q_D(TwitterAPI);

    QString statusCount = ( (msgCount > 200) ? QString::number(20) : QString::number(msgCount) );
    QString url = d->serviceUrl;
    url.append( urlStatuses );

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        QOAuth::ParamMap map;
        map.insert( "count", statusCount.toUtf8() );

        QByteArray parameters = d->prepareOAuthString( url, QOAuth::GET, map );

        request.setRawHeader( "Authorization", parameters );
        url.append( d->qoauth->inlineParameters( map, QOAuth::ParseForInlineQuery ) );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
        url.append( QString("?count=%1").arg( statusCount ) );
    }

    request.setUrl( QUrl(url) );
    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, role );
    request.setAttribute( TwitterAPIPrivate::ATTR_MSGCOUNT, statusCount );
}

/*!
  Sends a request for getting friends timeline.
  Length of the timeline can be adjusted by \a msgCount.

  \param msgCount Optional argument specifying length of requested timeline.
                  Twitter API currently accepts values up to 200.

  \sa newEntry(), publicTimeline()
*/
void TwitterAPI::friendsTimeline( int msgCount )
{
    Q_D(TwitterAPI);

    QNetworkRequest request;
    getTimelineRequest( request, TwitterAPIPrivate::UrlStatusesFriendsTimeline, TwitterAPI::ROLE_FRIENDS_TIMELINE, msgCount );
    qDebug() << "TwitterAPIPrivate::friendsTimeline(" + d->login + ")";

    d->iface->friendsInProgress = true;
    d->iface->connection.data()->get( request );
}

/*!
  Sends a request for getting mentions timeline (@user).
  Length of the timeline can be adjusted by \a msgCount.

  \param msgCount Optional argument specifying length of requested timeline.
                  Twitter API currently accepts values up to 200.

  \sa newEntry(), friendsTimeline()
*/
void TwitterAPI::mentions( int msgCount )
{
    Q_D(TwitterAPI);

    QNetworkRequest request;
    getTimelineRequest( request, TwitterAPIPrivate::UrlStatusesMentions, TwitterAPI::ROLE_MENTIONS, msgCount );
    qDebug() << "TwitterAPIPrivate::mentions(" + d->login + ")";

    d->iface->connection.data()->get( request );
}

/*!
  Sends a request for getting direct messages.
  Length of the timeline can be adjusted by \a msgCount.

  \param msgCount Optional argument specifying length of requested list.
                  Twitter API currently accepts values up to 200.

  \sa newEntry(), friendsTimeline()
*/
void TwitterAPI::directMessages( int msgCount )
{
    Q_D(TwitterAPI);

    QNetworkRequest request;
    getTimelineRequest( request, TwitterAPIPrivate::UrlDirectMessages, TwitterAPI::ROLE_DIRECT_MESSAGES, msgCount );
    request.setAttribute( TwitterAPIPrivate::ATTR_DM_REQUESTED, true );
    qDebug() << "TwitterAPI::directMessages(" + d->login + ")";

    if ( !d->iface->friendsInProgress ||
         d->iface->authorized ||
         d->iface->dmScheduled ) {
        d->iface->connection.data()->get( request );
        d->iface->dmScheduled = false;
    }
    else
        d->iface->dmScheduled = true;
}

/*!
  Not implemented yet.
*/
void TwitterAPI::postDM( const QString &screenName, const QString &text )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlDirectMessagesNew );

    QByteArray content;
    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        QOAuth::ParamMap map;
        map.insert( "user", screenName.toUtf8() );
        map.insert( "text", text.toUtf8().toPercentEncoding() );

        d->oauthForPost( request, url, map );
        content = d->qoauth->inlineParameters( map );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
        content = d->prepareRequest( screenName, text );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_POST_DM );
    request.setAttribute( TwitterAPIPrivate::ATTR_DM_RECIPIENT, screenName );
    request.setAttribute( TwitterAPIPrivate::ATTR_STATUS, text );

    qDebug() << "TwitterAPI::postDM(" << d->login << ")";
    d->iface->connection.data()->post( request, content );
}

/*!
  Already implemented, lol.
*/
void TwitterAPI::deleteDM( quint64 id )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlDirectMessagesDestroy.arg( QString::number(id) ) );

    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        d->oauthForPost( request, url );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_DELETE_DM );
    request.setAttribute( TwitterAPIPrivate::ATTR_ID, id );

    qDebug() << "TwitterAPI::deleteDM(" << d->login << ")";
    d->iface->connection.data()->post( request, QByteArray() );
}

/*!
  Sends a request for setting the status specified by \a id as a favorite.

  \param login User's login.
  \param password User's password.
  \param id An id of the status to be favorited.

*/
void TwitterAPI::createFavorite( quint64 id )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlFavoritesCreate.arg( QString::number(id) ) );

    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        d->oauthForPost( request, url );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_FAVORITES_CREATE );
    request.setAttribute( TwitterAPIPrivate::ATTR_ID, id );

    qDebug() << "TwitterAPI::createFavorite(" << d->login << ")";
    d->iface->connection.data()->post( request, QByteArray() );
}

/*!
  Sends a request for removing the status specified by \a id from favorites.

  \param login User's login.
  \param password User's password.
  \param id An id of the status to be removed from favorites.

*/
void TwitterAPI::destroyFavorite( quint64 id )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlFavoritesDestroy.arg( QString::number(id) ) );

    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        d->oauthForPost( request, url );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_FAVORITES_DESTROY );
    request.setAttribute( TwitterAPIPrivate::ATTR_ID, id );

    qDebug() << "TwitterAPI::destroyFavorite(" << d->login << ")";
    d->iface->connection.data()->post( request, QByteArray() );
}

/*!
  Sends a request for getting public timeline. Length of the timeline is fixed
  by Twitter API to 20 messages.

  \sa newEntry(), friendsTimeline()
*/
void TwitterAPI::publicTimeline()
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlStatusesPublicTimeline );

    QNetworkRequest request;
    request.setUrl( QUrl( url ) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_PUBLIC_TIMELINE );

    qDebug() << "TwitterAPI::publicTimeline()";
    d->iface->connection.data()->get( request );
}


void TwitterAPI::follow( quint64 userId )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlFriendshipCreate.arg( QString::number(userId) ) );

    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        d->oauthForPost( request, url );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_FRIENDSHIP_CREATE );
    request.setAttribute( TwitterAPIPrivate::ATTR_ID, userId );

    qDebug() << "TwitterAPI::follow(" << d->login << ")";
    d->iface->connection.data()->post( request, QByteArray() );
}

//void TwitterAPI::follow( const QString &userLogin )
//{
//}

void TwitterAPI::unfollow( quint64 userId )
{
    Q_D(TwitterAPI);

    QString url = d->serviceUrl;
    url.append( TwitterAPIPrivate::UrlFriendshipDestroy.arg( QString::number(userId) ) );

    QNetworkRequest request;

    if ( d->usingOAuth ) {
#ifdef HAVE_OAUTH
        d->oauthForPost( request, url );
#endif
    } else {
        request.setRawHeader( "Authorization", d->basicAuthString() );
    }

    request.setUrl( QUrl(url) );

    request.setAttribute( TwitterAPIPrivate::ATTR_ROLE, TwitterAPI::ROLE_FRIENDSHIP_DESTROY );
    request.setAttribute( TwitterAPIPrivate::ATTR_ID, userId );

    qDebug() << "TwitterAPI::unfollow(" << d->login << ")";
    d->iface->connection.data()->post( request, QByteArray() );
}

//void TwitterAPI::unfollow( const QString &userLogin )
//{
//}


/*!
  Resets all connections to Twitter.
*/
void TwitterAPI::resetConnections() {
    Q_D(TwitterAPI);

    d->iface->connection->deleteLater();
    d->iface->connection = new QNetworkAccessManager( this );
    connect( d->iface->connection, SIGNAL(finished(QNetworkReply*)), d, SLOT(requestFinished(QNetworkReply*)) );
    connect( d->iface->connection, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), d, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)) );
    if ( d->login != TwitterAPI::PUBLIC_TIMELINE ) {
        connect( d->iface->connection, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                 d, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)) );
    }
    d->iface->dmScheduled = false;
    d->iface->authorized = false;
    d->iface->friendsInProgress = false;
}

/*!
  Reads the reply code of a given \a reply, recognizes its type (with respect
  to actions supported by library) and performs appropriate operations, i.e.
  parses the reply and sends an appropriate signals.

  \param reply Reply received from Twitter API.
*/
void TwitterAPIPrivate::requestFinished( QNetworkReply *reply )
{
    Q_Q(TwitterAPI);
    //  qDebug() << "request finished";
    int replyCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    //  qDebug() << replyCode;
    if ( replyCode == 0 ) {
        reply->close();
        return;
    }
    QNetworkRequest request = reply->request();
    int role = request.attribute( TwitterAPIPrivate::ATTR_ROLE ).toInt();
    //  qDebug() << replyCode;
    //  QVariant login = request.attribute( TwitterAPIPrivate::ATTR_LOGIN );

    //  QString ntwk = (network==TwitterAPI::SOCIALNETWORK_TWITTER) ? "Twitter" : "Identi.ca";
    //  qDebug() << ntwk << login.toString();

    //  QVariant password = request.attribute( TwitterAPIPrivate::ATTR_PASSWORD );
    QVariant status = request.attribute( TwitterAPIPrivate::ATTR_STATUS );
    QVariant id = request.attribute( TwitterAPIPrivate::ATTR_ID );
    QVariant dm = request.attribute( TwitterAPIPrivate::ATTR_DM_REQUESTED );
    QVariant del = request.attribute( TwitterAPIPrivate::ATTR_DELETION_REQUESTED );
    QString searchString;
    switch ( replyCode ) {
    case 400: // temporary, weird Twitter behavior with deleting statuses
    case 200: // Ok
        if ( role && role != TwitterAPI::ROLE_PUBLIC_TIMELINE )
            iface->authorized = true;

        switch ( role ) {

        case TwitterAPI::ROLE_PUBLIC_TIMELINE:
            qDebug() << "TwitterAPI::requestFinished()" << "parsing public timeline";
            parseXml( reply->readAll(), ParseStatuses );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_FRIENDS_TIMELINE:
            qDebug() << "TwitterAPI::requestFinished()" << "parsing friends timeline";
            iface->friendsInProgress = false;
            if ( iface->dmScheduled )
            {
                if( int msgCount = request.attribute( TwitterAPIPrivate::ATTR_MSGCOUNT ).toInt() )
                    q->directMessages( msgCount );
            }
            parseXml( reply->readAll(), ParseStatuses );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_MENTIONS:
            qDebug() << "TwitterAPI::requestFinished()" << "parsing mentions timeline";
            parseXml( reply->readAll(), ParseStatuses );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_DIRECT_MESSAGES:
            qDebug() << "TwitterAPI::requestFinished()" << "parsing direct messages";
            parseXml( reply->readAll(), ParseDirectMessages );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_POST_UPDATE:
            parseXml( reply->readAll(), ParseStatuses );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_DELETE_UPDATE:
            emit q->deleteEntry( id.toULongLong() );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_POST_DM:
            emit q->postDMDone( TwitterAPI::ERROR_NO_ERROR );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_DELETE_DM:
            emit q->deleteDMDone( id.toULongLong(), TwitterAPI::ERROR_NO_ERROR );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_FAVORITES_CREATE:
            emit q->favoriteStatus( id.toULongLong(), true );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_FAVORITES_DESTROY:
            emit q->favoriteStatus( id.toULongLong(), false );
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_FRIENDSHIP_CREATE:
            searchString = QString( "<id>%1</id>" ).arg( id.toString() );
            if ( reply->readAll().contains( searchString.toUtf8() ) ) {
                emit q->followed( id.toULongLong() );
            }
            emit q->requestDone( role );
            break;

        case TwitterAPI::ROLE_FRIENDSHIP_DESTROY:
            searchString = QString( "<id>%1</id>" ).arg( id.toString() );
            if ( reply->readAll().contains( searchString.toUtf8() ) ) {
                emit q->unfollowed( id.toULongLong() );
            }
            emit q->requestDone( role );
            break;

        default:;
        }
        break;
    case 401:
        // Identi.ca works this way
        //    qDebug() << reply->readAll();
        emitUnauthorized( reply );
        break;
    case 403:
        switch ( role ) {
        case TwitterAPI::ROLE_FAVORITES_CREATE:
            // status is already favorite, TODO: emit a signal here in a future
            qDebug() << "[TwitterAPI] favorites/create: status already favorited";
            break;
        case TwitterAPI::ROLE_POST_DM:
            emit q->postDMDone( TwitterAPI::ERROR_DM_NOT_ALLOWED );
            qDebug() << "[TwitterAPI] direct_messages/new: you cannot send messages to users who are not following you, or recipient user not found.";
            break;
        default:;
        }
        emit q->requestDone( role );
        break;
    case 404: // Not Found
        qDebug() << "[TwitterAPI] error:" << replyCode;
        emit q->requestDone( role );
        break;
    case 502:
        if ( reply->operation() == QNetworkAccessManager::GetOperation ) {
            iface->connection.data()->get( request );
        }
        break;
    default:
        qDebug() << "[TwitterAPI] error:" << replyCode;
    }
    reply->close();
}

void TwitterAPIPrivate::parseXml( const QByteArray &data, TwitterAPIPrivate::ParsingMode mode )
{
    Q_Q(TwitterAPI);

    ParserRunnable *runnable = new ParserRunnable( q, data, mode );
    runnable->setAutoDelete(true);
    QThreadPool::globalInstance()->start( runnable );
}

#ifdef HAVE_OAUTH
QByteArray TwitterAPIPrivate::prepareOAuthString( const QString &requestUrl, QOAuth::HttpMethod method,
                                                  const QOAuth::ParamMap &params )
{
    int index = password.indexOf( '&' );
    QByteArray token = password.left( index ).toAscii();
    QByteArray tokenSecret = password.right( password.length() - index - 1 ).toAscii();
    QByteArray content = qoauth->createParametersString( requestUrl, method, token, tokenSecret,
                                                         QOAuth::HMAC_SHA1, params, QOAuth::ParseForHeaderArguments );
    return content;
}

void TwitterAPIPrivate::oauthForPost( QNetworkRequest &request, const QString &requestUrl,
                                      const QOAuth::ParamMap &params )
{
    QByteArray parameters = prepareOAuthString( requestUrl, QOAuth::POST, params );
    request.setRawHeader( "Authorization", parameters );
    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
}
#endif

QByteArray TwitterAPIPrivate::basicAuthString()
{
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    return auth.toBase64().prepend( "Basic " );
}

/*!
  Constructs a request from the given message text and optional \a inReplyTo argument.

  \param data Status message to be included in a request.
  \param inReplyTo Optional id of the status that the given status replies to.
*/
QByteArray TwitterAPIPrivate::prepareRequest( const QString &data, quint64 inReplyTo )
{
    QByteArray request( "status=" );
    request.append( data.toUtf8().toPercentEncoding() );
    if ( inReplyTo != 0 ) {
        request.append( "&in_reply_to_status_id=" + QByteArray::number( inReplyTo ) );
    }
    request.append( "&source=qtwitter" );
    return request;
}

/*!
  Constructs a request from the given message text and optional \a inReplyTo argument.

  \param data Status message to be included in a request.
  \param inReplyTo Optional id of the status that the given status replies to.
*/
QByteArray TwitterAPIPrivate::prepareRequest( const QString &screenName, const QString &text )
{
    QByteArray request( "user=" );
    request.append( screenName );
    request.append( "&text=" );
    request.append( text.toUtf8().toPercentEncoding() );
    return request;
}

/*!
  Executed upon a failed login to Twitter. Provides the authentication data,
  and when it doesn't fit, emits suitable unauthorized() signal.

  \param reply Reply received from Twitter API.
  \param authenticator A QAuthenticator object containing current (rejected)
                       authentication data.

  \sa unauthorized()
*/
void TwitterAPIPrivate::slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator )
{
    qDebug() << "auth required";

    QNetworkRequest request = reply->request();

#ifdef HAVE_OAUTH
    TwitterAPI::SocialNetwork network = (TwitterAPI::SocialNetwork) request.attribute( TwitterAPIPrivate::ATTR_SOCIALNETWORK ).toInt();
    if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
#endif
        QString login = request.attribute( TwitterAPIPrivate::ATTR_LOGIN ).toString();
        QString password = request.attribute( TwitterAPIPrivate::ATTR_PASSWORD ).toString();

        //    QString ntwk = (network==TwitterAPI::SOCIALNETWORK_TWITTER) ? "Twitter" : "Identi.ca";
        //    qDebug() << ntwk << login;

        if ( request.attribute( TwitterAPIPrivate::ATTR_DM_REQUESTED ).isValid() && // if this is the auth request for dm download
             iface->friendsInProgress ) { // and we're downloading friends timeline (i.e. authorising) just now
            reply->close();
            return;
        }
        if ( authenticator->user() != login || authenticator->password() != password ) {
            authenticator->setUser( login );
            authenticator->setPassword( password );
        }
        //  else {
        //    emitUnauthorized( reply );
        //  }
#ifdef HAVE_OAUTH
    }
#endif
}

void TwitterAPIPrivate::emitUnauthorized( QNetworkReply *reply )
{
    Q_Q(TwitterAPI);

    QNetworkRequest request = reply->request();

    TwitterAPI::Role role = (TwitterAPI::Role) request.attribute( TwitterAPIPrivate::ATTR_ROLE ).toInt();
    QVariant status = request.attribute( TwitterAPIPrivate::ATTR_STATUS );
    QVariant recipient = request.attribute( TwitterAPIPrivate::ATTR_DM_RECIPIENT );
    QVariant id = request.attribute( TwitterAPIPrivate::ATTR_ID );
    QVariant del = request.attribute( TwitterAPIPrivate::ATTR_DELETION_REQUESTED );

    // TODO: check if ATTR_DELETION_REQUESTED is needed
    if ( status.isValid() ) {
        switch ( role ) {
        case TwitterAPI::ROLE_POST_UPDATE:
            emit q->unauthorized( status.toString(), id.toULongLong() );
            break;
        case TwitterAPI::ROLE_POST_DM:
            emit q->unauthorized( recipient.toString(), status.toString() );
            break;
        default:;
        }
    } else if ( /*del.isValid() && del.toBool()*/ id.isValid() ) {
        switch ( role ) {
        case TwitterAPI::ROLE_DELETE_UPDATE:
            emit q->unauthorized( id.toULongLong(), Entry::Status );
            break;
        case TwitterAPI::ROLE_DELETE_DM:
            emit q->unauthorized( id.toULongLong(), Entry::DirectMessage );
            break;
        default:;
        }
    } else {
        emit q->unauthorized();
    }
    reply->abort();
    //    reply->close();
}
