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
  connect( xmlDownload, SIGNAL(unauthorized(QString,int)), this, SIGNAL(unauthorized(QString,int)) );
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

bool TwitterAPI::post( QString status, int inReplyTo )
{
  if ( authData.user().isEmpty() || authData.password().isEmpty() )
    return false;

  QByteArray request( "status=" );

  status.replace( QRegExp( "&" ), "%26" );
  status.replace( QRegExp( "\\+" ), "%2B" );

  request.append( status.toUtf8() );
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
  if ( entry->login == authData.user() ) {
    entry->isOwn = true;
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

/*! \class TwitterAPI
    \brief A class for interacting with Twitter API.

    This class is a frontend for communicating with Twitter REST API. Allows for
    getting either public or friends timeline, posting new status updates, deleting
    statuses, receiving direct messages, etc.
*/

/*! \enum TwitterAPI::Role
    Describes the function that the current connection has.
*/

/*! \var TwitterAPI::Role TwitterAPI::Refresh
    Statuses update is requested.
*/

/*! \var TwitterAPI::Role TwitterAPI::Submit
    Posting a new status is requested.
*/

/*! \var TwitterAPI::Role TwitterAPI::Destroy
    Destroying a Tweet is requested.
*/

/*! \enum TwitterAPI::ContentRequested
  Used to specify the content that is currently requested and has to be parsed.
*/

/*! \var TwitterAPI::ContentRequested TwitterAPI::Statuses
    Statuses are requested.
*/

/*! \var TwitterAPI::ContentRequested TwitterAPI::DirectMessages
    Direct messages are requested.
*/

/*! \fn TwitterAPI::TwitterAPI( QObject *parent = 0 )
    Creates a new TwitterAPI instance with a given \a parent.
*/

/*! \fn virtual TwitterAPI::~TwitterAPI()
    A virtual destructor.
*/

/*! \fn bool TwitterAPI::isPublicTimelineSync()
    Returns true if sync with public timeline is requested.
    \sa setPublicTimelineSync()
*/

/*! \fn bool TwitterAPI::isDirectMessagesSync()
    Returns true if direct messages downloading is requested.
    \sa setDirectMessagesSync()
*/

/*! \fn bool TwitterAPI::setAuthData( const QString &user, const QString &password )
    Sets user login and password for authentication at twitter.com.
    \param user User's login.
    \param password User's password.
*/

/*! \fn bool TwitterAPI::setPublicTimelineSync( bool b )
    Sets whether the public timeline is requested.
    \param b If true, a sync with public timeline is requested. If false, a sync with friends timeline is performed.
    \sa isPublicTimelineSync()
*/

/*! \fn bool TwitterAPI::setDirectMessagesSync( bool b )
    Sets whether drect messages are requested, when syncing with friends timeline. This setting has no effect if isPublicTimelineSync() returns true.
    \param b If true, direct messages are downloaded and added to friends timeline. If false, only friends' status updates are downloaded.
    \sa isDirectMessagesSync(), isPublicTimelineSync()
*/

/*! \fn bool TwitterAPI::get()
    Issues a timeline sync request, either public or friends one (with or without direct messages), according to
    values returned by isPublicTimelineSync and isDirectMessagesSync.
    \returns False when user's login and password are required and not provided, otherwise returns true.
    \sa post(), destroyTweet()
*/

/*! \fn bool TwitterAPI::post( const QByteArray &status, int inReplyTo = -1 )
    Sends a new Tweet with a content given by \a status.
    \param status New Tweet's text.
    \param inReplyTo In case the status is a reply - optional id of the existing status to which the reply is posted.
    \returns False if user's authenticaton data is missing, otherwise returns true.
    \sa get(), destroyTweet()
*/

/*! \fn bool TwitterAPI::destroyTweet( int id )
    Sends a request to delete Tweet of id given by \a id.
    \param id Id of the Tweet to be deleted.
    \returns False if user's authenticaton data is missing, otherwise returns true.
    \sa get(), post(), deleteEntry()
*/

/*! \fn void TwitterAPI::abort()
    Aborts the requests and closes open connections to Twitter.
*/

/*! \fn const QAuthenticator& TwitterAPI::getAuthData() const
    Outputs user's login and password.
    \returns QAuthenticator object containing user's authentication data.
*/

/*! \fn void TwitterAPI::setFlag( TwitterAPI::ContentRequested flag );
    Used to figure out when XmlDownload instance finishes its job. XmlDownload class emits
    signals connected to this slot when it finishes its requests. When all the requests are
    finished (i.e. one request when public timeline is requested or direct messages downloading
    is disabled or two requests when friends timeline with direct messages is requested), this
    slot resets connections and notifies User of new Tweets.
    \sa timelineUpdated()
*/

/*! \fn void TwitterAPI::errorMessage( const QString &message )
    Sends a \a message to notify user about encountered problems.
    \param message Error message.
*/

/*! \fn void TwitterAPI::authDataSet( const QAuthenticator &authenticator )
    Emitted when user authentication data changes.
    \param authenticator A QAuthenticator object containing new authentication data.
    \sa setAuthData()
*/

/*! \fn void TwitterAPI::unauthorized()
    Emitted when user is unauthorized to get timeline update.
    \sa unauthorized( const QByteArray &status, int inReplyToId ), unauthorized( int destroyId )
*/

/*! \fn void TwitterAPI::unauthorized( const QByteArray &status, int inReplyToId )
    Emitted when user is unauthorized to post a new status.
    \param status A status that was requested to be posted.
    \param inReplyToId Id of the status to which a reply was requested to be posted.
    \sa unauthorized(), unauthorized( int destroyId )
*/

/*! \fn void TwitterAPI::unauthorized( int destroyId )
    Emitted when user is unauthorized to delete a status.
    \param destroyId Id of the status to be destroyed.
    \sa unauthorized(), unauthorized( const QByteArray &status, int inReplyToId )
*/

/*! \fn void TwitterAPI::addEntry( Entry *entry )
    Emitted when a new status was parsed.
    \param entry A new status.
*/

/*! \fn void TwitterAPI::deleteEntry( int id )
    Emitted when a status was deleted.
    \param id Id of the deleted status.
*/

/*! \fn void TwitterAPI::requestListRefresh( bool isPublicTimeline, bool isSwitchUser)
    Emitted when user's request may possibly require deleting currently displayed list.
    \param isPublicTimeline Value returned by isPublicTimelineSync.
    \param isSwitchUser Indicates wether the user has changed since previous valid request.
    \sa isPublicTimelineSync()
*/

/*! \fn void TwitterAPI::done()
    Emitted when a single request has finished.
*/

/*! \fn void TwitterAPI::timelineUpdated()
    Emitted when all requests have finished.
*/

/*! \fn void TwitterAPI::directMessagesSyncChanged( bool isEnabled )
    Emitted to notify model that direct messages have been disabled or enabled,
    according to \a isEnabled.
    \param isEnabled Indicates if direct messages were enabled or disabled.
    \sa setDirectMessagesSync(), isDirectMessagesSync()
*/

/*! \fn void TwitterAPI::publicTimelineSyncChanged( bool isEnabled )
    Emitted when user switches to public timeline sync in authentication dialog.
    \sa isPublicTimelineSync(), setPublicTimelineSync()
*/

/*! \fn void TwitterAPI::userChanged()
    Emitted when authenticating user has changed.
*/
