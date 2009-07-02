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
#include "xmlparser.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QAuthenticator>
#include <QDebug>

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
  \fn void TwitterAPIInterface::requestDone( const QString &login, int role )

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
  \fn void TwitterAPIInterface::newEntry( const QString &login, Entry entry )

  Emitted when a new status or direct message is parsed from a Twitter API's
  reply for a request for the user identified by \a login. Used for both
  GET- and POST-type requests.

  \param login User's login.
  \param entry The parsed entry.

  \sa publicTimeline(), friendsTimeline(), directMessages(), postUpdate()
*/

/*!
  \fn void TwitterAPIInterface::deleteEntry( const QString &login, quint64 id )

  Emitted when a status of the given \a id was deleted from Twitter and
  can be deleted from locally stored list of the user specified by \a login.

  \param login User's login.
  \param id An id of the status to be deleted.

  \sa deleteUpdate()
*/

/*!
  \fn void TwitterAPIInterface::errorMessage( const QString &message )

  Emitted every time when something goes wrong with processing a request.
  This includes connection errors as well as Twitter internal errors.

  \param message An error message.
*/

/*!
  \fn void TwitterAPIInterface::unauthorized( const QString &login, const QString &password )

  Emitted when the given credentials are rejected by Twitter. Aborts the pending
  request immediately.

  Emitted for GET requests, i.e. friendsTimeline() and directMessages().

  \param login Login that was rejected.
  \param password Password that was rejected.

  \sa friendsTimeline(), directMessages()
*/

/*!
  \fn void TwitterAPIInterface::unauthorized( const QString &login, const QString &password, const QString &status, quint64 inReplyToStatusId )

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
  \fn void TwitterAPIInterface::unauthorized( const QString &login, const QString &password, int destroyId )

  Emitted when the given credentials are rejected by Twitter. Aborts the pending
  request immediately.

  Emitted for deleteUpdate() request, provides also details of the request,
  namely \a destroyId.

  \param login Login that was rejected.
  \param password Password that was rejected.
  \param destroyId Id of the status that was requested to be deleted.

  \sa deleteUpdate()
*/


const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_SOCIALNETWORK      = (QNetworkRequest::Attribute) QNetworkRequest::User;
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_ROLE               = (QNetworkRequest::Attribute) (QNetworkRequest::User + 1);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_LOGIN              = (QNetworkRequest::Attribute) (QNetworkRequest::User + 2);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_PASSWORD           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 3);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_STATUS             = (QNetworkRequest::Attribute) (QNetworkRequest::User + 4);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_STATUS_ID          = (QNetworkRequest::Attribute) (QNetworkRequest::User + 5);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_DM_REQUESTED       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 6);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_DM_RECIPIENT       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 7);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_DELETION_REQUESTED = (QNetworkRequest::Attribute) (QNetworkRequest::User + 8);
const QNetworkRequest::Attribute TwitterAPIInterface::ATTR_MSGCOUNT           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 9);

const QString TwitterAPIInterface::UrlStatusesPublicTimeline  = "/statuses/public_timeline.xml";
const QString TwitterAPIInterface::UrlStatusesFriendsTimeline = "/statuses/friends_timeline.xml";
const QString TwitterAPIInterface::UrlStatusesUpdate          = "/statuses/update.xml";
const QString TwitterAPIInterface::UrlStatusesDestroy         = "/statuses/destroy/%1.xml";
const QString TwitterAPIInterface::UrlDirectMessages          = "/direct_messages.xml";
const QString TwitterAPIInterface::UrlDirectMessagesNew       = "/direct_messages/new.xml";
const QString TwitterAPIInterface::UrlDirectMessagesDestroy   = "/direct_messages/destroy/%1.xml";
const QString TwitterAPIInterface::UrlFavoritesCreate         = "/favorites/create/%1.xml";
const QString TwitterAPIInterface::UrlFavoritesDestroy        = "/favorites/destroy/%1.xml";


/*!
  Constructs a new instance with a given \a parent.
*/
TwitterAPIInterface::TwitterAPIInterface( QObject *parent ) : QObject( parent )
{
  services[ TwitterAPI::SOCIALNETWORK_TWITTER ] = TwitterAPI::URL_TWITTER;
  services[ TwitterAPI::SOCIALNETWORK_IDENTICA ] = TwitterAPI::URL_IDENTICA;
  xmlReader = new QXmlSimpleReader;
  source = new QXmlInputSource;

#ifdef OAUTH
  qoauth = new QOAuth( this );
#endif
}

/*!
  A destructor.
*/
TwitterAPIInterface::~TwitterAPIInterface()
{
  if ( xmlReader ) {
    delete xmlReader;
    xmlReader = 0;
  }
  if ( source ) {
    delete source;
    source = 0;
  }

  QMap<QString,Interface*>::iterator i;
  foreach ( TwitterAPI::SocialNetwork network, connections.keys() ) {
    i = connections[ network ].begin();
    while ( i != connections[ network ].end() ) {
      delete (*i);
      i++;
    }
  }
}

#ifdef OAUTH
QByteArray TwitterAPIInterface::consumerKey() const
{
  return qoauth->consumerKey();
}

void TwitterAPIInterface::setConsumerKey( const QByteArray &consumerKey )
{
  qoauth->setConsumerKey( consumerKey );
}

QByteArray TwitterAPIInterface::consumerSecret() const
{
  return qoauth->consumerSecret();
}

void TwitterAPIInterface::setConsumerSecret( const QByteArray &consumerSecret )
{
  qoauth->setConsumerSecret( consumerSecret );
}
#endif

/*!
  Creates an interface (i.e. a QNetworkAccessManager instance and a set
  of XML parsers ) for connections to Twitter API for the user specified
  with \a login.

  \sa Interface
*/
Interface* TwitterAPIInterface::createInterface( TwitterAPI::SocialNetwork network, const QString &login )
{
  Interface *interface = new Interface;
  interface->connection = new QNetworkAccessManager( this );
  interface->statusParser = new XmlParser( network, login, this );

  interface->friendsInProgress = false;
  interface->authorized = false;
  interface->dmScheduled = false;

  if ( login != TwitterAPI::PUBLIC_TIMELINE ) {
    interface->directMsgParser = new XmlParserDirectMsg( network, login, this );
    connect( interface->directMsgParser, SIGNAL(newEntry(TwitterAPI::SocialNetwork,QString,Entry)), this, SIGNAL(newEntry(TwitterAPI::SocialNetwork,QString,Entry)) );
    connect( interface->connection, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)) );
  }
  connect( interface->connection, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
  connect( interface->statusParser, SIGNAL(newEntry(TwitterAPI::SocialNetwork,QString,Entry)), this, SIGNAL(newEntry(TwitterAPI::SocialNetwork,QString,Entry)) );
  connections[ network ].insert( login, interface );
  return interface;
}

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
void TwitterAPIInterface::postUpdate( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &data, quint64 inReplyTo )
{
  QString url = services.value(network);
  url.append( UrlStatusesUpdate );

  QByteArray content;
  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    QOAuth::ParamMap map;

    map.insert( "status", data.toUtf8().toPercentEncoding() );
    map.insert( "source", "qtwitter" );
    if ( inReplyTo != 0 ) {
      map.insert( "in_reply_to_status_id", QByteArray::number( inReplyTo ) );
    }

    content = prepareOAuthString( url, QOAuth::POST, password, map );

  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
    content = prepareRequest( data, inReplyTo );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  content = prepareRequest( data, inReplyTo );
#endif

  request.setUrl( QUrl(url) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_POST_UPDATE );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_STATUS, data );
  request.setAttribute( TwitterAPIInterface::ATTR_STATUS_ID, inReplyTo );
  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  qDebug() << "TwitterAPIInterface::postUpdate(" + login + ")";
  connections[ network ][ login ]->connection.data()->post( request, content );
}

/*!
  Sends a request to delete a status of a given \a id for the user identified
  by \a login and \a password.

  \param login User's login.
  \param password User's password.
  \param id An id of the status to be deleted.

  \sa deleteEntry(), postUpdate()
*/
void TwitterAPIInterface::deleteUpdate( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id )
{
  QString url = services.value(network);
  url.append( UrlStatusesDestroy.arg( QString::number(id) ) );

  QByteArray content;

  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    content = prepareOAuthString( url, QOAuth::POST, password );
  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
#endif

  request.setUrl( QUrl(url) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_DELETE_UPDATE );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_DELETION_REQUESTED, true );
  request.setAttribute( TwitterAPIInterface::ATTR_STATUS_ID, id );
  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  qDebug() << "TwitterAPIInterface::deleteUpdate(" + login + ")";
  connections[ network ][ login ]->connection.data()->post( request, content );
}

/*!
  Sends a request for getting friends timeline for the user identified
  by \a login and \a password. Length of the timeline can be adjusted by
  \a msgCount.

  \param login User's login.
  \param password User's password.
  \param msgCount Optional argument specifying length of requested timeline.
                  Twitter API currently accepts values up to 200.

  \sa newEntry(), publicTimeline()
*/
void TwitterAPIInterface::friendsTimeline( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, int msgCount )
{
  QString statusCount = ( (msgCount > 200) ? QString::number(20) : QString::number(msgCount) );

  QString url = services.value(network);
  url.append( UrlStatusesFriendsTimeline );

  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    QOAuth::ParamMap map;
    map.insert( "count", statusCount.toUtf8() );

    QByteArray parameters = prepareOAuthString( url, QOAuth::GET, password, map );

    url.append( parameters );
  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
    url.append( QString("?count=%1").arg( statusCount ) );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  url.append( QString("?count=%1").arg( statusCount ) );
#endif

  request.setUrl( QUrl(url) );
  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_FRIENDS_TIMELINE );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_MSGCOUNT, statusCount );
  qDebug() << "TwitterAPIInterface::friendsTimeline(" + login + ")";

  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  connections[ network ][ login ]->friendsInProgress = true;
  connections[ network ][ login ]->connection.data()->get( request );
}

/*!
  Sends a request for getting direct messages for the user identified
  by \a login and \a password. Length of the timeline can be adjusted by
  \a msgCount.

  \param login User's login.
  \param password User's password.
  \param msgCount Optional argument specifying length of requested list.
                  Twitter API currently accepts values up to 200.

  \sa newEntry(), friendsTimeline()
*/
void TwitterAPIInterface::directMessages( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, int msgCount )
{
  QString statusCount = ( (msgCount > 200) ? QString::number(20) : QString::number(msgCount) );

  QString url = services.value(network);
  url.append( UrlDirectMessages );

  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    QOAuth::ParamMap map;
    map.insert( "count", statusCount.toUtf8() );

    QByteArray parameters = prepareOAuthString( url, QOAuth::GET, password, map );
    url.append( parameters );
  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
    url.append( QString("?count=%1").arg( statusCount ) );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  url.append( QString("?count=%1").arg( statusCount ) );
#endif
  request.setUrl( QUrl(url) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_DIRECT_MESSAGES );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_DM_REQUESTED, true );
  qDebug() << "TwitterAPIInterface::directMessages(" + login + ")";

  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  if ( !connections[ network ][ login ]->friendsInProgress ||
       connections[ network ][ login ]->authorized ||
       connections[ network ][ login ]->dmScheduled ) {
    connections[ network ][ login ]->connection.data()->get( request );
    connections[ network ][ login ]->dmScheduled = false;
  }
  else
    connections[ network ][ login ]->dmScheduled = true;
}

/*!
  Not implemented yet.
*/
void TwitterAPIInterface::postDM( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &screenName, const QString &text )
{
  QString url = services.value(network);
  url.append( UrlDirectMessagesNew );

  QByteArray content;
  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    QOAuth::ParamMap map;
    map.insert( "user", screenName.toUtf8() );
    map.insert( "text", text.toUtf8().toPercentEncoding() );

    content = prepareOAuthString( url, QOAuth::POST, password, map );

  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
    content = prepareRequest( screenName, text );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  content = prepareRequest( screenName, text );
#endif

  request.setUrl( QUrl(url) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_POST_DM );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_DM_RECIPIENT, screenName );
  request.setAttribute( TwitterAPIInterface::ATTR_STATUS, text );

  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  qDebug() << "TwitterAPIInterface::postDM(" << login << ")";
  connections[ network ][ login ]->connection.data()->post( request, content );
}

/*!
  Not implemented yet.
*/
void TwitterAPIInterface::deleteDM( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id )
{
  QString url = services.value(network);
  url.append( UrlDirectMessagesDestroy.arg( QString::number(id) ) );

  QByteArray content;
  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    content = prepareOAuthString( url, QOAuth::POST, password );
  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
#endif

  request.setUrl( QUrl(url) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_DELETE_DM );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_STATUS_ID, id );

  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  qDebug() << "TwitterAPIInterface::deleteDM(" << login << ")";
  connections[ network ][ login ]->connection.data()->post( request, content );
}

/*!
  Sends a request for setting the status specified by \a id as a favorite.

  \param login User's login.
  \param password User's password.
  \param id An id of the status to be favorited.

*/
void TwitterAPIInterface::createFavorite( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id )
{
  QString url = services.value(network);
  url.append( UrlFavoritesCreate.arg( QString::number(id) ) );

  QByteArray content;
  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    content = prepareOAuthString( url, QOAuth::POST, password );
  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
#endif

  request.setUrl( QUrl(url) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_FAVORITES_CREATE );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_STATUS_ID, id );
  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  qDebug() << "TwitterAPIInterface::createFavorite(" << login << ")";
  connections[ network ][ login ]->connection.data()->post( request, content );
}

/*!
  Sends a request for removing the status specified by \a id from favorites.

  \param login User's login.
  \param password User's password.
  \param id An id of the status to be removed from favorites.

*/
void TwitterAPIInterface::destroyFavorite( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 id )
{
  QString url = services.value(network);
  url.append( UrlFavoritesDestroy.arg( QString::number(id) ) );

  QByteArray content;
  QNetworkRequest request;

#ifdef OAUTH
  if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
    content = prepareOAuthString( url, QOAuth::POST, password );
  } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
    request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
  }
#else
  QByteArray auth = login.toUtf8() + ":" + password.toUtf8();
  request.setRawHeader( "Authorization", "Basic " + auth.toBase64() );
#endif

  request.setUrl( QUrl(url) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_FAVORITES_DESTROY );
  request.setAttribute( TwitterAPIInterface::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPIInterface::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPIInterface::ATTR_STATUS_ID, id );
  if ( !connections[ network ].contains( login ) )
    createInterface( network, login );
  qDebug() << "TwitterAPIInterface::destroyFavorite(" << login << ")";
  connections[ network ][ login ]->connection.data()->post( request, content );
}

/*!
  Sends a request for getting public timeline. Length of the timeline is fixed
  by Twitter API to 20 messages.

  \sa newEntry(), friendsTimeline()
*/
void TwitterAPIInterface::publicTimeline( TwitterAPI::SocialNetwork network )
{
  QString url = services.value(network);
  url.append( UrlStatusesPublicTimeline );

  QNetworkRequest request;
  request.setUrl( QUrl( url ) );

  request.setAttribute( TwitterAPIInterface::ATTR_SOCIALNETWORK, network );
  request.setAttribute( TwitterAPIInterface::ATTR_ROLE, TwitterAPI::ROLE_PUBLIC_TIMELINE );
  if ( !connections[ network ].contains( TwitterAPI::PUBLIC_TIMELINE ) )
    createInterface( network, TwitterAPI::PUBLIC_TIMELINE );
  qDebug() << "TwitterAPIInterface::publicTimeline()";
  connections[ network ][ TwitterAPI::PUBLIC_TIMELINE ]->connection.data()->get( request );
}


/*!
  Resets all connections to Twitter.
*/
void TwitterAPIInterface::resetConnections() {
  QMap<QString,Interface*>::iterator i;
  foreach ( TwitterAPI::SocialNetwork network, connections.keys() ) {
    i = connections[ network ].begin();
    while ( i != connections[ network ].end() ) {
      (*i)->connection->deleteLater();
      (*i)->connection = new QNetworkAccessManager( this );
      connect( (*i)->connection, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
      if ( i.key() != TwitterAPI::PUBLIC_TIMELINE )
        connect( (*i)->connection, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)) );
      (*i)->dmScheduled = false;
      (*i)->authorized = false;
      (*i)->friendsInProgress = false;
      i++;
    }
  }
}

/*!
  Parses the XML document given by \a data using \a parser.

  \param data The XML document to be parsed.
  \param parser The parser to parse the data.

  \sa requestFinished()
*/
void TwitterAPIInterface::parseXml( const QByteArray &data, XmlParser *parser )
{
  source->setData( data );
  xmlReader->setContentHandler( parser );
  xmlReader->parse( source );
}


/*!
  Reads the reply code of a given \a reply, recognizes its type (with respect
  to actions supported by library) and performs appropriate operations, i.e.
  parses the reply and sends an appropriate signals.

  \param reply Reply received from Twitter API.
*/
void TwitterAPIInterface::requestFinished( QNetworkReply *reply )
{
  qDebug() << "request finished";
  int replyCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  qDebug() << replyCode;
  if ( replyCode == 0 ) {
    reply->close();
    return;
  }
  QNetworkRequest request = reply->request();
  int role = request.attribute( TwitterAPIInterface::ATTR_ROLE ).toInt();
  TwitterAPI::SocialNetwork network = (TwitterAPI::SocialNetwork)request.attribute( TwitterAPIInterface::ATTR_SOCIALNETWORK ).toInt();
//  qDebug() << replyCode;
  QVariant login = request.attribute( TwitterAPIInterface::ATTR_LOGIN );
//  QString ntwk = (network==TwitterAPI::SOCIALNETWORK_TWITTER) ? "Twitter" : "Identi.ca";
//  qDebug() << ntwk << login.toString();
  QVariant password = request.attribute( TwitterAPIInterface::ATTR_PASSWORD );
  QVariant status = request.attribute( TwitterAPIInterface::ATTR_STATUS );
  QVariant id = request.attribute( TwitterAPIInterface::ATTR_STATUS_ID );
  QVariant dm = request.attribute( TwitterAPIInterface::ATTR_DM_REQUESTED );
  QVariant del = request.attribute( TwitterAPIInterface::ATTR_DELETION_REQUESTED );
  switch ( replyCode ) {
  case 200: // Ok
    if ( role && role != TwitterAPI::ROLE_PUBLIC_TIMELINE )
      connections[ network ][ login.toString() ]->authorized = true;

    switch ( role ) {

    case TwitterAPI::ROLE_PUBLIC_TIMELINE:
      qDebug() << "TwitterAPIInterface::requestFinished()" << "parsing public timeline";
      parseXml( reply->readAll(), connections[ network ][ TwitterAPI::PUBLIC_TIMELINE ]->statusParser );
      emit requestDone( network, TwitterAPI::PUBLIC_TIMELINE, role );
      break;

    case TwitterAPI::ROLE_FRIENDS_TIMELINE:
      qDebug() << "TwitterAPIInterface::requestFinished()" << "parsing friends timeline";
      connections[ network ][ login.toString() ]->friendsInProgress = false;
      if ( connections[ network ][ login.toString() ]->dmScheduled )
      {
        if( int msgCount = request.attribute( TwitterAPIInterface::ATTR_MSGCOUNT ).toInt() )
          directMessages( network, login.toString(), password.toString() , msgCount);
      }
      parseXml( reply->readAll(), connections[ network ][ login.toString() ]->statusParser );
      emit requestDone( network, login.toString(), role );
      break;

    case TwitterAPI::ROLE_DIRECT_MESSAGES:
      qDebug() << "TwitterAPIInterface::requestFinished()" << "parsing direct messages";
      parseXml( reply->readAll(), connections[ network ][ login.toString() ]->directMsgParser );
      emit requestDone( network, login.toString(), role );
      break;

    case TwitterAPI::ROLE_POST_UPDATE:
      parseXml( reply->readAll(), connections[ network ][ login.toString() ]->statusParser );
      emit requestDone( network, login.toString(), role );
      break;

    case TwitterAPI::ROLE_DELETE_UPDATE:
      emit deleteEntry( network, login.toString(), id.toULongLong() );
      emit requestDone( network, login.toString(), role );
      break;

    case TwitterAPI::ROLE_POST_DM:
      emit postDMDone( network, login.toString(), TwitterAPI::ERROR_NO_ERROR );
      emit requestDone( network, login.toString(), role );
      break;

    case TwitterAPI::ROLE_DELETE_DM:
      emit deleteDMDone( network, login.toString(), id.toULongLong(), TwitterAPI::ERROR_NO_ERROR );
      emit requestDone( network, login.toString(), role );
      break;

    case TwitterAPI::ROLE_FAVORITES_CREATE:
      emit favoriteStatus( network, login.toString(), id.toULongLong(), true );
      emit requestDone( network, login.toString(), role );
      break;

    case TwitterAPI::ROLE_FAVORITES_DESTROY:
      emit favoriteStatus( network, login.toString(), id.toULongLong(), false );
      emit requestDone( network, login.toString(), role );
      break;

    default:;
    }
    break;
  case 401:
    // Identi.ca works this way
    emitUnauthorized( reply );
    break;
  case 403:
    switch ( role ) {
    case TwitterAPI::ROLE_FAVORITES_CREATE:
      // status is already favorite, TODO: emit a signal here in a future
      qDebug() << "[TwitterAPI] favorites/create: status already favorited";
      break;
    case TwitterAPI::ROLE_POST_DM:
      emit postDMDone( network, login.toString(), TwitterAPI::ERROR_DM_NOT_ALLOWED );
      qDebug() << "[TwitterAPI] direct_messages/new: you cannot send messages to users who are not following you, or recipient user not found.";
      break;
    default:;
    }
    if ( login.isValid() )
      emit requestDone( network, login.toString(), role );
    else
      emit requestDone( network, TwitterAPI::PUBLIC_TIMELINE, role );
    break;
  case 404: // Not Found
    qDebug() << "[TwitterAPI] error:" << replyCode;
    emit requestDone( network, login.toString(), role );
    break;
  case 502:
    if ( reply->operation() == QNetworkAccessManager::GetOperation ) {
      if ( login.isValid() )
        connections[ network ][ login.toString() ]->connection.data()->get( request );
      else
        connections[ network ][ TwitterAPI::PUBLIC_TIMELINE ]->connection.data()->get( request );
    }
    break;
  default:
    qDebug() << "[TwitterAPI] error:" << replyCode;
//    if ( login.isValid() )
//      emit requestDone( network, login.toString(), role );
//    else
//      emit requestDone( network, TwitterAPI::PUBLIC_TIMELINE, role );
  }
  reply->close();
}

#ifdef OAUTH
QByteArray TwitterAPIInterface::prepareOAuthString( const QString &requestUrl, QOAuth::HttpMethod method,
                                                    const QString &password, const QOAuth::ParamMap &params )
{
  int index = password.indexOf( '&' );
  QByteArray token = password.left( index ).toAscii();
  QByteArray tokenSecret = password.right( password.length() - index - 1 ).toAscii();
  QByteArray content = qoauth->createParametersString( requestUrl, method, token, tokenSecret,
                                                       QOAuth::HMAC_SHA1, params, QOAuth::ParseForInlineQuery );
  return content;
}
#endif

/*!
  Constructs a request from the given message text and optional \a inReplyTo argument.

  \param data Status message to be included in a request.
  \param inReplyTo Optional id of the status that the given status replies to.
*/
QByteArray TwitterAPIInterface::prepareRequest( const QString &data, quint64 inReplyTo )
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
QByteArray TwitterAPIInterface::prepareRequest( const QString &screenName, const QString &text )
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
void TwitterAPIInterface::slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator )
{
  qDebug() << "auth required";
  QNetworkRequest request = reply->request();

  TwitterAPI::SocialNetwork network = (TwitterAPI::SocialNetwork) request.attribute( TwitterAPIInterface::ATTR_SOCIALNETWORK ).toInt();
  QString login = request.attribute( TwitterAPIInterface::ATTR_LOGIN ).toString();
  QString password = request.attribute( TwitterAPIInterface::ATTR_PASSWORD ).toString();

//  QString ntwk = (network==TwitterAPI::SOCIALNETWORK_TWITTER) ? "Twitter" : "Identi.ca";
//  qDebug() << ntwk << login;

  if ( request.attribute( TwitterAPIInterface::ATTR_DM_REQUESTED ).isValid() && // if this is the auth request for dm download
       connections[ network ][ login ]->friendsInProgress ) { // and we're downloading friends timeline (i.e. authorising) just now
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
}

void TwitterAPIInterface::emitUnauthorized( QNetworkReply *reply )
{
  QNetworkRequest request = reply->request();

  TwitterAPI::Role role = (TwitterAPI::Role) request.attribute( TwitterAPIInterface::ATTR_ROLE ).toInt();
  TwitterAPI::SocialNetwork network = (TwitterAPI::SocialNetwork) request.attribute( TwitterAPIInterface::ATTR_SOCIALNETWORK ).toInt();
  QString login = request.attribute( TwitterAPIInterface::ATTR_LOGIN ).toString();
  QString password = request.attribute( TwitterAPIInterface::ATTR_PASSWORD ).toString();
  QVariant status = request.attribute( TwitterAPIInterface::ATTR_STATUS );
  QVariant recipient = request.attribute( TwitterAPIInterface::ATTR_DM_RECIPIENT );
  QVariant id = request.attribute( TwitterAPIInterface::ATTR_STATUS_ID );
  QVariant del = request.attribute( TwitterAPIInterface::ATTR_DELETION_REQUESTED );

  // TODO: check if ATTR_DELETION_REQUESTED is needed
  if ( status.isValid() ) {
    switch ( role ) {
    case TwitterAPI::ROLE_POST_UPDATE:
      emit unauthorized( network, login, password, status.toString(), id.toULongLong() );
      break;
    case TwitterAPI::ROLE_POST_DM:
      emit unauthorized( network, login, password, recipient.toString(), status.toString() );
      break;
    default:;
    }
  } else if ( /*del.isValid() && del.toBool()*/ id.isValid() ) {
    switch ( role ) {
    case TwitterAPI::ROLE_DELETE_UPDATE:
      emit unauthorized( network, login, password, id.toULongLong(), Entry::Status );
      break;
    case TwitterAPI::ROLE_DELETE_DM:
      emit unauthorized( network, login, password, id.toULongLong(), Entry::DirectMessage );
      break;
    default:;
    }
  } else {
    emit unauthorized( network, login, password );
  }
  reply->abort();
  //    reply->close();
}
