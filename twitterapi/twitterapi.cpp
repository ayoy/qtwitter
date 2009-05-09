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


#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QAuthenticator>
#include <QDebug>
#include "twitterapi.h"
#include "xmlparser.h"

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
  \class TwitterAPI
  \brief A class interacting with Twitter.

  This class provides an interface for connecting to Twitter REST API.
  Its usage boils down to instantiating it and issuing requests for retrieving
  needed data, like publicTimeline(), friendsTimeline() or directMessages().

  Once a timeline or direct messages GET or POST request completes successfully,
  the recived XML document is parsed. The newEntry() signal is emitted with every
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
  \enum TwitterAPI::Role

  Set for sent requests, represents the activity performed by the specific request.
*/

/*!
  \var TwitterAPI::ROLE_PUBLIC_TIMELINE

  Indicates that the request is for downloading the public timeline.
*/

/*!
  \var TwitterAPI::ROLE_FRIENDS_TIMELINE

  Indicates that the request is for downloading the friends timeline.
*/

/*!
  \var TwitterAPI::ROLE_DIRECT_MESSAGES

  Indicates that the request is for downloading direct messages list.
*/

/*!
  \var TwitterAPI::ROLE_POST_UPDATE

  Indicates that the request is for posting a status update.
*/

/*!
  \var TwitterAPI::ROLE_DELETE_UPDATE

  Indicates that the request is for deleting a status update.
*/

/*!
  \var TwitterAPI::ROLE_POST_DM

  Indicates that the request is for posting a direct message.

  \note Not implemented yet.
*/

/*!
  \var TwitterAPI::ROLE_DELETE_DM

  Indicates that the request is for deleting a direct message.

  \note Not implemented yet.
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
  \fn void TwitterAPI::deleteEntry( const QString &login, int id )

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
  \fn void TwitterAPI::unauthorized( const QString &login, const QString &password, const QString &status, int inReplyToId )

  Emitted when the given credentials are rejected by Twitter. Aborts the pending
  request immediately.

  Emitted for postUpdate() request, provides also details of the request,
  like \a status and \a inReplyToId.

  \param login Login that was rejected.
  \param password Password that was rejected.
  \param status Status message that was requested to be sent.
  \param inReplyToId Optional id of the message that the requested status is replying to.

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

/*!
  A constant used as a "login" for public timeline requests.
*/
const QString TwitterAPI::PUBLIC_TIMELINE = "public timeline";

const QNetworkRequest::Attribute TwitterAPI::ATTR_ROLE               = (QNetworkRequest::Attribute) QNetworkRequest::User;
const QNetworkRequest::Attribute TwitterAPI::ATTR_LOGIN              = (QNetworkRequest::Attribute) (QNetworkRequest::User + 1);
const QNetworkRequest::Attribute TwitterAPI::ATTR_PASSWORD           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 2);
const QNetworkRequest::Attribute TwitterAPI::ATTR_STATUS             = (QNetworkRequest::Attribute) (QNetworkRequest::User + 3);
const QNetworkRequest::Attribute TwitterAPI::ATTR_INREPLYTO_ID       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 4);
const QNetworkRequest::Attribute TwitterAPI::ATTR_DM_REQUESTED       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 5);
const QNetworkRequest::Attribute TwitterAPI::ATTR_DELETION_REQUESTED = (QNetworkRequest::Attribute) (QNetworkRequest::User + 6);
const QNetworkRequest::Attribute TwitterAPI::ATTR_DELETE_ID          = (QNetworkRequest::Attribute) (QNetworkRequest::User + 7);
const QNetworkRequest::Attribute TwitterAPI::ATTR_MSGCOUNT           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 8);

/*!
  Constructs a new instance with a given \a parent.
*/
TwitterAPI::TwitterAPI( QObject *parent ) : QObject( parent )
{
  xmlReader = new QXmlSimpleReader;
  source = new QXmlInputSource;
}

/*!
  A destructor.
*/
TwitterAPI::~TwitterAPI()
{
  if ( xmlReader ) {
    delete xmlReader;
    xmlReader = 0;
  }
  if ( source ) {
    delete source;
    source = 0;
  }
  QMap<QString,Interface*>::iterator i = connections.begin();
  while ( i != connections.end() ) {
    delete (*i);
    i++;
  }
}

/*!
  Creates an interface (i.e. a QNetworkAccessManager instance and a set
  of XML parsers ) for connections to Twitter API for the user specified
  with \a login.

  \sa Interface
*/
Interface* TwitterAPI::createInterface( const QString &login )
{
  Interface *interface = new Interface;
  interface->connection = new QNetworkAccessManager( this );
  interface->statusParser = new XmlParser( login, this );

  interface->friendsInProgress = false;
  interface->authorized = false;
  interface->dmScheduled = false;

  if ( login != TwitterAPI::PUBLIC_TIMELINE ) {
    interface->directMsgParser = new XmlParserDirectMsg( login, this );
    connect( interface->directMsgParser, SIGNAL(newEntry(QString,Entry)), this, SIGNAL(newEntry(QString,Entry)) );
    connect( interface->connection, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)) );
  }
  connect( interface->connection, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
  connect( interface->statusParser, SIGNAL(newEntry(QString,Entry)), this, SIGNAL(newEntry(QString,Entry)) );
  connections.insert( login, interface );
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
void TwitterAPI::postUpdate( const QString &login, const QString &password, const QString &data, int inReplyTo )
{
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/update.xml" ) );
  QByteArray content = prepareRequest( data, inReplyTo );
  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_POST_UPDATE );
  request.setAttribute( TwitterAPI::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPI::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPI::ATTR_STATUS, data );
  request.setAttribute( TwitterAPI::ATTR_INREPLYTO_ID, inReplyTo );
  if ( !connections.contains( login ) )
    createInterface( login );
  qDebug() << "TwitterAPI::postUpdate(" + login + ")";
  connections[ login ]->connection.data()->post( request, content );
}

/*!
  Sends a request to delete a status of a given \a id for the user identified
  by \a login and \a password.

  \param login User's login.
  \param password User's password.
  \param id An id of the status to be deleted.

  \sa deleteEntry(), postUpdate()
*/
void TwitterAPI::deleteUpdate( const QString &login, const QString &password, int id )
{
  QNetworkRequest request( QUrl( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ) ) );
  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_DELETE_UPDATE );
  request.setAttribute( TwitterAPI::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPI::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPI::ATTR_DELETION_REQUESTED, true );
  request.setAttribute( TwitterAPI::ATTR_DELETE_ID, id );
  if ( !connections.contains( login ) )
    createInterface( login );
  qDebug() << "TwitterAPI::deleteUpdate(" + login + ")";
  connections[ login ]->connection.data()->post( request, QByteArray() );
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
void TwitterAPI::friendsTimeline( const QString &login, const QString &password, int msgCount )
{
  QString tweetCount = ( (msgCount > 200) ? QString::number(20) : QString::number(msgCount) );
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/friends_timeline.xml?count="+tweetCount) );
  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_FRIENDS_TIMELINE );
  request.setAttribute( TwitterAPI::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPI::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPI::ATTR_MSGCOUNT, tweetCount );
  qDebug() << "TwitterAPI::friendsTimeline(" + login + ")";
  if ( !connections.contains( login ) )
    createInterface( login );
  connections[ login ]->friendsInProgress = true;
  connections[ login ]->connection.data()->get( request );
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
void TwitterAPI::directMessages( const QString &login, const QString &password, int msgCount )
{
  /* When directMessages is called from requestFinished, msgCount argument shouldn't be out of bounds,
     but we can check if the value is correct anyway
  */
  QString tweetCount = ( (msgCount > 200) ? QString::number(20) : QString::number(msgCount) );
  QNetworkRequest request( QUrl( "http://twitter.com/direct_messages.xml?count="+tweetCount) );
  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_DIRECT_MESSAGES );
  request.setAttribute( TwitterAPI::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPI::ATTR_PASSWORD, password );
  request.setAttribute( TwitterAPI::ATTR_DM_REQUESTED, true );
  qDebug() << "TwitterAPI::directMessages(" + login + ")";
  if ( !connections.contains( login ) )
    createInterface( login );
  if ( !connections[ login ]->friendsInProgress || connections[ login ]->authorized || connections[ login ]->dmScheduled ) {
    connections[ login ]->connection.data()->get( request );
    connections[ login ]->dmScheduled = false;
  }
  else
    connections[ login ]->dmScheduled = true;
}

/*!
  Not implemented yet.
*/
void TwitterAPI::postDM( const QString &login, const QString &password, const QString &user, const QString &data )
{
  Q_UNUSED(login);
  Q_UNUSED(password);
  Q_UNUSED(user);
  Q_UNUSED(data);
//  QNetworkRequest request( QUrl( "http://twitter.com/statuses/update.xml" ) );
//  QByteArray content = prepareRequest( data, inReplyTo );
//  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_POST_DM );
//  request.setAttribute( TwitterAPI::ATTR_LOGIN, login );
//  request.setAttribute( TwitterAPI::ATTR_PASSWORD, password );
//  request.setAttribute( TwitterAPI::ATTR_STATUS, data );
//  if ( !connections.contains( login ) )
//    createInterface( login );
//  qDebug() << "TwitterAPI::postDM(" + login + ")";
//  connections[ login ]->connection.data()->post( request, content );
}

/*!
  Not implemented yet.
*/
void TwitterAPI::deleteDM( const QString &login, const QString &password, int id )
{
  Q_UNUSED(login);
  Q_UNUSED(password);
  Q_UNUSED(id);
//  QNetworkRequest request( QUrl( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ) ) );
//  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_DELETE_DM );
//  request.setAttribute( TwitterAPI::ATTR_LOGIN, login );
//  request.setAttribute( TwitterAPI::ATTR_PASSWORD, password );
//  request.setAttribute( TwitterAPI::ATTR_DELETION_REQUESTED, true );
//  request.setAttribute( TwitterAPI::ATTR_DELETE_ID, id );
//  if ( !connections.contains( login ) )
//    createInterface( login );
//  qDebug() << "TwitterAPI::deleteDM(" + login + ")";
//  connections[ login ]->connection.data()->post( request, QByteArray() );
}

/*!
  Sends a request for getting public timeline. Length of the timeline is fixed
  by Twitter API to 20 messages.

  \sa newEntry(), friendsTimeline()
*/
void TwitterAPI::publicTimeline()
{
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/public_timeline.xml" ) );
  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_PUBLIC_TIMELINE );
  if ( !connections.contains( TwitterAPI::PUBLIC_TIMELINE ) )
    createInterface( TwitterAPI::PUBLIC_TIMELINE );
  qDebug() << "TwitterAPI::publicTimeline()";
  connections[ TwitterAPI::PUBLIC_TIMELINE ]->connection.data()->get( request );
}

/*!
  Resets all connections to Twitter.
*/
void TwitterAPI::resetConnections() {
  QMap<QString,Interface*>::iterator i = connections.begin();
  while ( i != connections.end() ) {
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

/*!
  Parses the XML document given by \a data using \a parser.

  \param data The XML document to be parsed.
  \param parser The parser to parse the data.

  \sa requestFinished()
*/
void TwitterAPI::parseXml( const QByteArray &data, XmlParser *parser )
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
void TwitterAPI::requestFinished( QNetworkReply *reply )
{
  int replyCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  if ( replyCode == 0 ) {
    reply->close();
    return;
  }
  QNetworkRequest request = reply->request();
  int role = request.attribute( TwitterAPI::ATTR_ROLE ).toInt();
  QVariant login = request.attribute( TwitterAPI::ATTR_LOGIN );
  QVariant password = request.attribute( TwitterAPI::ATTR_PASSWORD );
  QVariant status = request.attribute( TwitterAPI::ATTR_STATUS );
  QVariant inreplyto = request.attribute( TwitterAPI::ATTR_INREPLYTO_ID );
  QVariant dm = request.attribute( TwitterAPI::ATTR_DM_REQUESTED );
  QVariant del = request.attribute( TwitterAPI::ATTR_DELETION_REQUESTED );
  QVariant delId = request.attribute( TwitterAPI::ATTR_DELETE_ID );
  switch ( replyCode ) {
  case 200: // Ok
    if ( role && role != TwitterAPI::ROLE_PUBLIC_TIMELINE )
      connections[ login.toString() ]->authorized = true;

    switch ( role ) {

    case TwitterAPI::ROLE_PUBLIC_TIMELINE:
      qDebug() << "TwitterAPI::requestFinished()" << "parsing public timeline";
      parseXml( reply->readAll(), connections[ TwitterAPI::PUBLIC_TIMELINE ]->statusParser );
      emit requestDone( TwitterAPI::PUBLIC_TIMELINE, TwitterAPI::ROLE_PUBLIC_TIMELINE );
      break;

    case TwitterAPI::ROLE_FRIENDS_TIMELINE:
      qDebug() << "TwitterAPI::requestFinished()" << "parsing friends timeline";
      connections[ login.toString() ]->friendsInProgress = false;
      if ( connections[ login.toString() ]->dmScheduled )
      {
        if( int msgCount = request.attribute( TwitterAPI::ATTR_MSGCOUNT ).toInt() )
          directMessages( login.toString(), password.toString() , msgCount);
      }
      parseXml( reply->readAll(), connections[ login.toString() ]->statusParser );
      emit requestDone( login.toString(), TwitterAPI::ROLE_FRIENDS_TIMELINE );
      break;

    case TwitterAPI::ROLE_DIRECT_MESSAGES:
      qDebug() << "TwitterAPI::requestFinished()" << "parsing direct messages";
      parseXml( reply->readAll(), connections[ login.toString() ]->directMsgParser );
      emit requestDone( login.toString(), TwitterAPI::ROLE_DIRECT_MESSAGES );
      break;

    case TwitterAPI::ROLE_POST_UPDATE:
      parseXml( reply->readAll(), connections[ login.toString() ]->statusParser );
      emit requestDone( login.toString(), TwitterAPI::ROLE_POST_UPDATE );
      break;

    case TwitterAPI::ROLE_DELETE_UPDATE:
      emit deleteEntry( login.toString(), delId.toInt() );
      emit requestDone( login.toString(), TwitterAPI::ROLE_DELETE_UPDATE );
      break;

    case TwitterAPI::ROLE_POST_DM:
      break;

    case TwitterAPI::ROLE_DELETE_DM:
      break;

    default:;
    }
    break;
  case 404: // Not Found
    emit errorMessage( "Not found" );
    break;
  case 502:
    if ( reply->operation() == QNetworkAccessManager::GetOperation ) {
      if ( login.isValid() )
        connections[ login.toString() ]->connection.data()->get( request );
      else
        connections[ TwitterAPI::PUBLIC_TIMELINE ]->connection.data()->get( request );
    }
  default:;
  }
  reply->close();
}

/*!
  Constructs a request from the given message text and optional \a inReplyTo argument.

  \param data Status message to be included in a request.
  \param inReplyTo Optional id of the status that the given status replies to.
*/
QByteArray TwitterAPI::prepareRequest( const QString &data, int inReplyTo )
{
  QByteArray request( "status=" );
  QString statusText( data );
  statusText.replace( QRegExp( "&" ), "%26" );
  statusText.replace( QRegExp( "\\+" ), "%2B" );
  request.append( data.toUtf8() );
  if ( inReplyTo != -1 ) {
    request.append( "&in_reply_to_status_id=" + QByteArray::number( inReplyTo ) );
  }
  request.append( "&source=qtwitter" );
  qDebug() << request;
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
void TwitterAPI::slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator )
{
  qDebug() << "auth required";
  QNetworkRequest request = reply->request();
  qDebug() << request.attribute( TwitterAPI::ATTR_DM_REQUESTED ).toBool();

  QString login = request.attribute( TwitterAPI::ATTR_LOGIN ).toString();
  QString password = request.attribute( TwitterAPI::ATTR_PASSWORD ).toString();

  if ( request.attribute( TwitterAPI::ATTR_DM_REQUESTED ).isValid() && // if this is the auth request for dm download
       connections[ login ]->friendsInProgress ) { // and we're downloading friends timeline (i.e. authorising) just now
    reply->close();
    return;
  }
  if ( authenticator->user() != login || authenticator->password() != password ) {
    authenticator->setUser( login );
    authenticator->setPassword( password );
  } else {
    QVariant status = request.attribute( TwitterAPI::ATTR_STATUS );
    QVariant inreplyto = request.attribute( TwitterAPI::ATTR_INREPLYTO_ID );
    QVariant del = request.attribute( TwitterAPI::ATTR_DELETION_REQUESTED );
    QVariant delId = request.attribute( TwitterAPI::ATTR_DELETE_ID );

    if ( del.isValid() && del.toBool() ) {
      emit unauthorized( login, password, delId.toInt() );
    } else if ( status.isValid() ) {
      emit unauthorized( login, password, status.toString(), inreplyto.toBool() );
    } else {
      emit unauthorized( login, password );
    }
    reply->abort();
//    reply->close();
  }
}
