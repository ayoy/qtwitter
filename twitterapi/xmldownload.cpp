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


#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "xmldownload.h"
#include "xmlparser.h"
#include "xmlparserdirectmsg.h"

const QNetworkRequest::Attribute XmlDownload::ATTR_LOGIN              = (QNetworkRequest::Attribute) QNetworkRequest::User;
const QNetworkRequest::Attribute XmlDownload::ATTR_PASSWORD           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 1);
const QNetworkRequest::Attribute XmlDownload::ATTR_STATUS             = (QNetworkRequest::Attribute) (QNetworkRequest::User + 2);
const QNetworkRequest::Attribute XmlDownload::ATTR_INREPLYTO_ID       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 3);
const QNetworkRequest::Attribute XmlDownload::ATTR_DM_REQUESTED       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 4);
const QNetworkRequest::Attribute XmlDownload::ATTR_DELETION_REQUESTED = (QNetworkRequest::Attribute) (QNetworkRequest::User + 5);
const QNetworkRequest::Attribute XmlDownload::ATTR_DELETE_ID          = (QNetworkRequest::Attribute) (QNetworkRequest::User + 6);

XmlDownload::XmlDownload( QObject *parent ) :
    QObject( parent ),
    statusParser( new XmlParser( this ) ),
    directMsgParser( new XmlParserDirectMsg( this ) )
{
  connections.insert( TwitterAPI::PUBLIC_TIMELINE, new QNetworkAccessManager( this ) );
  connect( connections[ TwitterAPI::PUBLIC_TIMELINE ], SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
  connect( statusParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
  connect( directMsgParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
}

XmlDownload::~XmlDownload()
{
  QMap<QString,QNetworkAccessManager*>::iterator i = connections.begin();
  while ( i != connections.end() ) {
    (*i)->deleteLater();
    i++;
  }
  statusParser->deleteLater();
  directMsgParser->deleteLater();
}

void XmlDownload::createConnections()
{
//  connect( directMsgParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
//  connect( this, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QString,quint16,QAuthenticator*)));
}

QNetworkAccessManager* XmlDownload::createNetworkAccessManager( const QString &login )
{
  QNetworkAccessManager *connection = new QNetworkAccessManager( this );
  connect( connection, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
  connect( connection, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)) );
  connections.insert( login, connection );
  authStatuses.insert( login, false );
  return connection;
}

void XmlDownload::postUpdate( const QString &login, const QString &password, const QString &data, int inReplyTo )
{
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/update.xml" ) );
  QByteArray content = prepareRequest( data, inReplyTo );
  request.setAttribute( XmlDownload::ATTR_LOGIN, login );
  request.setAttribute( XmlDownload::ATTR_PASSWORD, password );
  request.setAttribute( XmlDownload::ATTR_STATUS, data );
  request.setAttribute( XmlDownload::ATTR_INREPLYTO_ID, inReplyTo );
  if ( !connections.contains( login ) )
    createNetworkAccessManager( login );
  connections[ login ]->post( request, content );
}

void XmlDownload::deleteUpdate( const QString &login, const QString &password, int id )
{
  QNetworkRequest request( QUrl( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ) ) );
  request.setAttribute( XmlDownload::ATTR_LOGIN, login );
  request.setAttribute( XmlDownload::ATTR_PASSWORD, password );
  request.setAttribute( XmlDownload::ATTR_DELETION_REQUESTED, true );
  request.setAttribute( XmlDownload::ATTR_DELETE_ID, id );
  if ( !connections.contains( login ) )
    createNetworkAccessManager( login );
  connections[ login ]->post( request, QByteArray() );
}

void XmlDownload::friendsTimeline( const QString &login, const QString &password )
{
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/friends_timeline.xml" ) );
  request.setAttribute( XmlDownload::ATTR_LOGIN, login );
  request.setAttribute( XmlDownload::ATTR_PASSWORD, password );
  qDebug() << "XmlDownload::friendsTimeline(" + login + "," + password + ")";
  qDebug() << setAuthorizationData( login, password );
  if ( !connections.contains( login ) )
    createNetworkAccessManager( login );
  connections[ login ]->get( request );
}

void XmlDownload::directMessages( const QString &login, const QString &password )
{
  QNetworkRequest request( QUrl( "http://twitter.com/direct_messages.xml" ) );
  request.setAttribute( XmlDownload::ATTR_LOGIN, login );
  request.setAttribute( XmlDownload::ATTR_PASSWORD, password );
  request.setAttribute( XmlDownload::ATTR_DM_REQUESTED, true );
  qDebug() << "XmlDownload::directMessages(" + login + "," + password + ")";
  qDebug() << setAuthorizationData( login, password );
  if ( !connections.contains( login ) )
    createNetworkAccessManager( login );
  connections[ login ]->get( request );
}

void XmlDownload::publicTimeline()
{
  connections[ TwitterAPI::PUBLIC_TIMELINE ]->get( QNetworkRequest( QUrl( "http://twitter.com/statuses/public_timeline.xml" ) ) );
}

void XmlDownload::parseXml( const QByteArray &data, XmlParser *parser )
{
  source.setData( data );
  xmlReader.setContentHandler( parser );
  xmlReader.parse( source );
}

void XmlDownload::requestFinished( QNetworkReply *reply )
{
  int replyCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  qDebug() << replyCode;
  if ( replyCode == 0 ) {
    reply->close();
    return;
  }
  QNetworkRequest request = reply->request();
  QVariant login = request.attribute( XmlDownload::ATTR_LOGIN );
  QVariant password = request.attribute( XmlDownload::ATTR_PASSWORD );
  qDebug() << "XmlDownload::requestFinished(" + login.toString() + "," + password.toString() + ")";
  QVariant status = request.attribute( XmlDownload::ATTR_STATUS );
  QVariant inreplyto = request.attribute( XmlDownload::ATTR_INREPLYTO_ID );
  QVariant dm = request.attribute( XmlDownload::ATTR_DM_REQUESTED );
  QVariant del = request.attribute( XmlDownload::ATTR_DELETION_REQUESTED );
  QVariant delId = request.attribute( XmlDownload::ATTR_DELETE_ID );
  switch ( replyCode ) {
  case 200: // Ok
    if ( reply->operation() == QNetworkAccessManager::GetOperation ) {
      if ( login.isValid() ) {
        if ( dm.isValid() && dm.toBool() ) {
          parseXml( reply->readAll(), directMsgParser );
        } else {
          parseXml( reply->readAll(), statusParser );
        }
      } else {
          parseXml( reply->readAll(), statusParser );
      }
    } else if ( reply->operation() == QNetworkAccessManager::PostOperation ) {
      if ( login.isValid() ) {
        if ( del.isValid() && del.toBool() ) {
          emit deleteEntry( delId.toInt() );
        } else {
          parseXml( reply->readAll(), statusParser );
        }
      }
    }
    break;
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  case 404:                   // Not Found
    emit errorMessage( "Not found" );
    break;
  case 401:
    break;
  default:;
  }
  reply->close();
}

QByteArray XmlDownload::prepareRequest( const QString &data, int inReplyTo )
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

QByteArray XmlDownload::setAuthorizationData( const QString &login, const QString &password )
{
  QByteArray auth( login.toUtf8() );
  auth.append(":");
  auth.append( password.toUtf8() );
  auth = auth.toBase64();
  return auth.prepend( "Basic " );
}

void XmlDownload::slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator )
{
  qDebug() << "auth required";

  QNetworkRequest request = reply->request();
//  reply->close();

  QString login = request.attribute( XmlDownload::ATTR_LOGIN ).toString();
  QString password = request.attribute( XmlDownload::ATTR_PASSWORD ).toString();

  if ( authenticator->user() != login || authenticator->password() != password ) {
    authenticator->setUser( login );
    authenticator->setPassword( password );
  } else {
    QVariant status = request.attribute( XmlDownload::ATTR_STATUS );
    QVariant inreplyto = request.attribute( XmlDownload::ATTR_INREPLYTO_ID );
    QVariant del = request.attribute( XmlDownload::ATTR_DELETION_REQUESTED );
    QVariant delId = request.attribute( XmlDownload::ATTR_DELETE_ID );

    if ( del.isValid() && del.toBool() ) {
      emit unauthorized( delId.toInt() );
    } else if ( status.isValid() ) {
      emit unauthorized( status.toString(), inreplyto.toBool() );
    } else {
      emit unauthorized();
    }
  }
}
