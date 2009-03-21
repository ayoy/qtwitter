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


#include "twitterapi.h"
#include "xmldownload.h"
#include "entry.h"

TwitterAPI::TwitterAPI( QObject *parent ) :
    QObject( parent ),
    publicTimelineSync( false ),
    directMessagesSync( false ),
    switchUser( false ),
    authDialogOpen( false ),
    xmlGet( NULL ),
    xmlPost( NULL ),
    statusesDone( false ),
    messagesDone( false )
{
}

TwitterAPI::~TwitterAPI() {}

void TwitterAPI::createConnections( XmlDownload *xmlDownload )
{
  connect( xmlDownload, SIGNAL(finished(TwitterAPI::ContentRequested)), this, SLOT(setFlag(TwitterAPI::ContentRequested)) );
  connect( xmlDownload, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  connect( xmlDownload, SIGNAL(unauthorized()), this, SIGNAL(unauthorized()) );
  connect( xmlDownload, SIGNAL(unauthorized(QByteArray,int)), this, SIGNAL(unauthorized(QByteArray,int)) );
  connect( xmlDownload, SIGNAL(unauthorized(int)), this, SIGNAL(unauthorized(int)) );
  if ( xmlDownload->getRole() == TwitterAPI::Destroy ) {
    connect( xmlDownload, SIGNAL(deleteEntry(int)), this, SIGNAL(deleteEntry(int)) );
  } else {
    connect( xmlDownload, SIGNAL(newEntry(Entry*)), this, SLOT(newEntry(Entry*)) );
  }
}

bool TwitterAPI::isPublicTimelineSync()
{
  return publicTimelineSync;
}

bool TwitterAPI::isDirectMessagesSync()
{
  return directMessagesSync;
}

bool TwitterAPI::setAuthData( const QString &user, const QString &password )
{
  switchUser = false;
  authData.setUser( user );
  authData.setPassword( password );
  if ( currentUser.isNull() ) {
    currentUser = user;
  } else if ( currentUser.compare( authData.user() ) ) {
    switchUser = true;
    emit userChanged();
  }
  emit requestListRefresh( publicTimelineSync, switchUser );
  return switchUser;
}

bool TwitterAPI::setPublicTimelineSync( bool b )
{
  if ( publicTimelineSync != b ) {
    publicTimelineSync = b;
    emit publicTimelineSyncChanged( b );
    return true;
  }
  return false;
}

bool TwitterAPI::setDirectMessagesSync( bool b )
{
  if ( directMessagesSync != b ) {
    directMessagesSync = b;
    emit directMessagesSyncChanged( b );
    return true;
  }
  return false;
}

bool TwitterAPI::get()
{
  if ( publicTimelineSync ) {
    xmlGet = new XmlDownload ( TwitterAPI::Refresh, authData.user(), authData.password(), this );
    createConnections( xmlGet );
    xmlGet->getContent( "http://twitter.com/statuses/public_timeline.xml", TwitterAPI::Statuses );
  } else {
    if ( authData.user().isEmpty() || authData.password().isEmpty() )
      return false;

    xmlGet = new XmlDownload ( TwitterAPI::Refresh, authData.user(), authData.password(), this );
    createConnections( xmlGet );
    xmlGet->getContent( "http://twitter.com/statuses/friends_timeline.xml", TwitterAPI::Statuses );
    if ( directMessagesSync ) {
      xmlGet->getContent( "http://twitter.com/direct_messages.xml", TwitterAPI::DirectMessages );
    }
  }
  emit requestListRefresh( publicTimelineSync, switchUser );
  switchUser = false;
  return true;
}

bool TwitterAPI::post( const QByteArray &status, int inReplyTo )
{
  if ( authData.user().isEmpty() || authData.password().isEmpty() )
    return false;

  QByteArray request( "status=" );
  request.append( status );
  if ( inReplyTo != -1 ) {
    request.append( "&in_reply_to_status_id=" + QByteArray::number( inReplyTo ) );
  }
  request.append( "&source=qtwitter" );
  qDebug() << request;
  xmlPost = new XmlDownload( TwitterAPI::Submit, authData.user(), authData.password(), this );
  createConnections( xmlPost );
  xmlPost->setPostStatus( status );
  xmlPost->setPostInReplyToId( inReplyTo );
  xmlPost->postContent( "http://twitter.com/statuses/update.xml", request, TwitterAPI::Statuses );
  emit requestListRefresh( publicTimelineSync, switchUser );
  switchUser = false;
  return true;
}

bool TwitterAPI::destroyTweet( int id )
{
  if ( authData.user().isEmpty() || authData.password().isEmpty() )
    return false;

  qDebug() << "Tweet No." << id << "will be destroyed";
  xmlPost = new XmlDownload( TwitterAPI::Destroy, authData.user(), authData.password(), this );
  createConnections( xmlPost );
  xmlPost->setDestroyId( id );
  xmlPost->postContent( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ), QByteArray(), TwitterAPI::Statuses );
  emit requestListRefresh( publicTimelineSync, switchUser );
  switchUser = false;
  return true;
}

void TwitterAPI::abort()
{
  if ( xmlPost ) {
    xmlPost->abort();
  }
  if ( xmlGet ) {
    xmlGet->abort();
  }
  destroyXmlConnection();
}

const QAuthenticator& TwitterAPI::getAuthData() const
{
  return authData;
}

void TwitterAPI::setFlag( TwitterAPI::ContentRequested flag )
{
  switch ( flag ) {
    case TwitterAPI::DirectMessages:
      messagesDone = true;
      break;
    case TwitterAPI::Statuses:
    default:
      statusesDone = true;
  }
  emit done();
  emit publicTimelineSyncChanged( publicTimelineSync );
  if ( statusesDone && ( publicTimelineSync || (!directMessagesSync ? true : messagesDone) || (xmlPost && !publicTimelineSync)  ) ) {
    emit timelineUpdated();
    emit authDataSet( authData );
    destroyXmlConnection();
    currentUser = authData.user();
    statusesDone = false;
    messagesDone = false;
  }
}

void TwitterAPI::newEntry( Entry *entry )
{
  if ( entry->login() == authData.user() ) {
    entry->setOwn( true );
  }
  emit addEntry( entry );
}

void TwitterAPI::destroyXmlConnection()
{
  if ( xmlPost ) {
    qDebug() << "destroying xmlPost";
    xmlPost->deleteLater();
    xmlPost = NULL;
  }
  if ( xmlGet ) {
    qDebug() << "destroying xmlGet";
    xmlGet->deleteLater();
    xmlGet = NULL;
  }
}
