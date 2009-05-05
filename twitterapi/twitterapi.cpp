/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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
#include <QEventLoop>
#include <QAuthenticator>
#include "twitterapi.h"
#include "xmlparser.h"

const QString TwitterAPI::PUBLIC_TIMELINE = "public timeline";

const QNetworkRequest::Attribute TwitterAPI::ATTR_ROLE               = (QNetworkRequest::Attribute) QNetworkRequest::User;
const QNetworkRequest::Attribute TwitterAPI::ATTR_LOGIN              = (QNetworkRequest::Attribute) (QNetworkRequest::User + 1);
const QNetworkRequest::Attribute TwitterAPI::ATTR_PASSWORD           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 2);
const QNetworkRequest::Attribute TwitterAPI::ATTR_STATUS             = (QNetworkRequest::Attribute) (QNetworkRequest::User + 3);
const QNetworkRequest::Attribute TwitterAPI::ATTR_INREPLYTO_ID       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 4);
const QNetworkRequest::Attribute TwitterAPI::ATTR_DM_REQUESTED       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 5);
const QNetworkRequest::Attribute TwitterAPI::ATTR_DELETION_REQUESTED = (QNetworkRequest::Attribute) (QNetworkRequest::User + 6);
const QNetworkRequest::Attribute TwitterAPI::ATTR_DELETE_ID          = (QNetworkRequest::Attribute) (QNetworkRequest::User + 7);

TwitterAPI::TwitterAPI( QObject *parent ) : QObject( parent ) {}

TwitterAPI::~TwitterAPI()
{
  QMap<QString,Interface*>::iterator i = connections.begin();
  while ( i != connections.end() ) {
    delete (*i);
    i++;
  }
}

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

void TwitterAPI::friendsTimeline( const QString &login, const QString &password, int msgCount)
{
  QString tweetCount = ( (msgCount > 200) ? QString::number(20) : QString::number(msgCount));
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/friends_timeline.xml?count="+tweetCount));
  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_FRIENDS_TIMELINE );
  request.setAttribute( TwitterAPI::ATTR_LOGIN, login );
  request.setAttribute( TwitterAPI::ATTR_PASSWORD, password );
  qDebug() << "TwitterAPI::friendsTimeline(" + login + ")";
  if ( !connections.contains( login ) )
    createInterface( login );
  connections[ login ]->friendsInProgress = true;
  connections[ login ]->connection.data()->get( request );
}

void TwitterAPI::directMessages( const QString &login, const QString &password )
{
  QNetworkRequest request( QUrl( "http://twitter.com/direct_messages.xml" ) );
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

void postDM( const QString &login, const QString &password, const QString &data )
{
  Q_UNUSED(login);
  Q_UNUSED(password);
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

void deleteDM( const QString &login, const QString &password, int id )
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

void TwitterAPI::publicTimeline()
{
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/public_timeline.xml" ) );
  request.setAttribute( TwitterAPI::ATTR_ROLE, TwitterAPI::ROLE_PUBLIC_TIMELINE );
  if ( !connections.contains( TwitterAPI::PUBLIC_TIMELINE ) )
    createInterface( TwitterAPI::PUBLIC_TIMELINE );
  qDebug() << "TwitterAPI::publicTimeline()";
  connections[ TwitterAPI::PUBLIC_TIMELINE ]->connection.data()->get( request );
}

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

void TwitterAPI::parseXml( const QByteArray &data, XmlParser *parser )
{
  source.setData( data );
  xmlReader.setContentHandler( parser );
  xmlReader.parse( source );
}

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
        directMessages( login.toString(), password.toString() );
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
      connections[ login.toString() ]->authorized = true;
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
