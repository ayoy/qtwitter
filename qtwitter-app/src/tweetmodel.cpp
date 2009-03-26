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


#include "tweetmodel.h"
#include "entry.h"
#include "tweet.h"
#include "mainwindow.h"

TweetModel::TweetModel( int margin, StatusList *parentListView, QObject *parent ) :
  QStandardItemModel( 0, 0, parent ),
  modelToBeCleared( false ),
  newStatusesCount( 0 ),
  newMessagesCount( 0 ),
  maxTweetCount( 20 ),
  scrollBarMargin( margin ),
  currentIndex( QModelIndex() ),
  view( parentListView )
{
  connect( view, SIGNAL(clicked(QModelIndex)), this, SLOT(selectTweet(QModelIndex)) );
  connect( view, SIGNAL(moveFocus(bool)), this, SLOT(moveFocus(bool)) );
  Tweet::setTweetListModel( this );
}

void TweetModel::deselectCurrentIndex()
{
  if ( currentIndex != QModelIndex() ) {
    Tweet *aTweet = getTweetFromIndex( currentIndex );
    if ( aTweet->isRead() ) {
      aTweet->setState( Tweet::Read );
    } else {
      aTweet->setState( Tweet::Unread );
    }
    currentIndex = QModelIndex();
  }
}

void TweetModel::setTheme( const ThemeData &theme )
{
  Tweet::setTheme( theme );
  if ( rowCount() > 0 ) {
    for ( int i = 0; i < rowCount(); i++ ) {
      Tweet *aTweet = getTweetFromIndex( i );
      aTweet->applyTheme();
    }
  }
}

void TweetModel::setMaxTweetCount( int count )
{
  maxTweetCount = count;
  stripRedundantTweets();
}

void TweetModel::insertTweet( Entry *entry )
{
  if ( modelToBeCleared ) {
    clear();
    modelToBeCleared = false;
  }
  for ( int i = 0; i < rowCount(); ++i ) {
    if ( entry->id == item(i)->data().value<Entry>().id ) {
      qDebug() << "found existing entry of the same id";
      return;
    }
  }
  QVariant data = qVariantFromValue( *entry );
  QStandardItem *newItem = new QStandardItem();
  newItem->setData( data );
  Tweet *newTweet;
  if ( entry->type == Entry::DirectMessage )
    newTweet = new Tweet( *entry, QImage( ":/icons/mail_48.png" ), dynamic_cast<MainWindow*>( parent()) );
  else
    newTweet = new Tweet( *entry, QImage(), dynamic_cast<MainWindow*>( parent()) );
  newTweet->resize( view->width() - scrollBarMargin, newTweet->size().height() );
  newItem->setSizeHint( newTweet->size() );

  if ( rowCount() == 0 ) {
    QStandardItemModel::appendRow( newItem );
    view->setIndexWidget( indexFromItem( newItem ), newTweet );
    return;
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( entry->timestamp > item(i)->data().value<Entry>().timestamp ) {
      QStandardItemModel::insertRow( i, newItem );
      view->setIndexWidget( indexFromItem( newItem ), newTweet );
      if ( currentIndex.row() >= i && currentIndex.row() < (maxTweetCount - 1) )
        selectTweet( currentIndex.sibling( currentIndex.row() + 1, 0 ) );
      stripRedundantTweets();
      return;
    }
  }
  if ( stripRedundantTweets() ) {
    newTweet->deleteLater();
    return;
  }
  QStandardItemModel::appendRow( newItem );
  view->setIndexWidget( indexFromItem( newItem ), newTweet );
}

void TweetModel::deleteTweet( int id )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( id == item(i)->data().value<Entry>().id  ) {
      removeRow( i );
      return;
    }
  }
}

void TweetModel::slotDirectMessagesChanged( bool isEnabled )
{
  if ( !isEnabled )
    return;
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( item(i)->data().value<Entry>().type == Entry::DirectMessage ) {
      removeRow( i );
      i--;
    }
  }
}

void TweetModel::selectTweet( const QModelIndex &index )
{
  Tweet *aTweet;
  if ( currentIndex != QModelIndex() ) {
    aTweet = getTweetFromIndex( currentIndex );
    if ( aTweet->isRead() )
      aTweet->setState( Tweet::Read );
    else
      aTweet->setState( Tweet::Unread );
  }
  currentIndex = index;
  aTweet = getTweetFromIndex( currentIndex );
  aTweet->setState( Tweet::Active );
  view->setCurrentIndex( currentIndex );
}

void TweetModel::selectTweet( Tweet *tweet )
{
  if ( currentIndex != QModelIndex() ) {
    Tweet *aTweet = getTweetFromIndex( currentIndex );
    if ( aTweet->isRead() )
      aTweet->setState( Tweet::Read );
    else
      aTweet->setState( Tweet::Unread );
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *matchTweet = getTweetFromIndex( i );
    if ( matchTweet == tweet ) {
      currentIndex = indexFromItem( item(i) );
      break;
    }
  }
  tweet->setState( Tweet::Active );
  view->setCurrentIndex( currentIndex );
}

void TweetModel::markAllAsRead()
{
  if ( rowCount() > 0 ) {
    for ( int i = 0; i < rowCount(); i++ ) {
      Tweet *aTweet = getTweetFromIndex( i );
      if ( i == currentIndex.row() )
        aTweet->setState( Tweet::Active );
      else
        aTweet->setState( Tweet::Read );
    }
  }
}

void TweetModel::sendNewsInfo()
{
  countUnreadEntries();
  emit newTweets( newStatusesCount, newStatusesNames, newMessagesCount, newMessagesNames );
  newStatusesCount = newMessagesCount = 0;
  publicTimeline = publicTimelineRequested;
  newStatusesNames.clear();
  newMessagesNames.clear();
}

void TweetModel::retranslateUi()
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = getTweetFromIndex( i );
    aTweet->retranslateUi();
  }
}

void TweetModel::resizeData( int width, int oldWidth )
{
  if ( rowCount() == 0 )
    return;

  QSize itemSize;
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = getTweetFromIndex( i );
    aTweet->resize( width - scrollBarMargin, aTweet->size().height() );
    itemSize = item(i)->sizeHint();
    itemSize.rwidth() += width - oldWidth;
    itemSize.rheight() = aTweet->size().height();
    item(i)->setSizeHint( itemSize );
  }
}

void TweetModel::moveFocus( bool up )
{
  if ( !rowCount() )
    return;
  if ( currentIndex == QModelIndex() ) {
    currentIndex = this->index( 0, 0 );
    selectTweet( currentIndex );
    return;
  }
  if ( up ) {
    if ( currentIndex.row() > 0 ) {
      selectTweet( currentIndex.sibling( currentIndex.row() - 1, 0 ) );
    }
  } else {
    if ( currentIndex.row() < rowCount() - 1 ) {
      selectTweet( currentIndex.sibling( currentIndex.row() + 1, 0 ) );
    }
  }
}

void TweetModel::setImageForUrl( const QString& url, QImage image )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = getTweetFromIndex( i );
    if ( url == item(i)->data().value<Entry>().image ) {
      aTweet->setIcon( image );
    }
  }
}

void TweetModel::setModelToBeCleared( bool wantsPublic, bool userChanged )
{
  bool timelineChanged = (!publicTimeline && wantsPublic) || (publicTimeline && !wantsPublic);
  if ( (!publicTimeline && !timelineChanged && !userChanged) || (publicTimeline && !timelineChanged) ) {
    qDebug() << publicTimeline << timelineChanged << userChanged << "won't clear list";
    modelToBeCleared = false;
    publicTimelineRequested = wantsPublic;
    return;
  }
  qDebug() << publicTimeline << wantsPublic << userChanged << "will clear list";
  deselectCurrentIndex();
  modelToBeCleared = true;
  publicTimelineRequested = wantsPublic;
}

void TweetModel::setPublicTimelineRequested( bool b )
{
  publicTimelineRequested = b;
}

void TweetModel::countUnreadEntries()
{
  for ( int i = 0; i < rowCount(); ++i ) {
    if ( !getTweetFromIndex( i )->isRead() ) {
      addUnreadEntry( item(i)->data().value<Entry>() );
    }
  }
}

void TweetModel::addUnreadEntry( Entry entry )
{
  switch ( entry.type ) {
  case Entry::DirectMessage:
    newMessagesCount += 1;
    if ( !newMessagesNames.contains( entry.name ) ) {
      newMessagesNames << entry.name;
    }
    break;
  case Entry::Status:
  default:
    newStatusesCount += 1;
    QString name = entry.isOwn ? tr( "you" ) : entry.name;
    if ( !newStatusesNames.contains( name ) ) {
      newStatusesNames << name;
    }
  }
}

bool TweetModel::stripRedundantTweets()
{
  if ( rowCount() >= maxTweetCount ) {
    if ( currentIndex.row() > maxTweetCount - 1 ) {
      selectTweet( currentIndex.sibling( maxTweetCount - 1, 0 ) );
    }
    QStandardItemModel::removeRows( maxTweetCount, rowCount() - maxTweetCount );
    return true;
  }
  return false;
}

Tweet* TweetModel::getTweetFromIndex( int i )
{
  return dynamic_cast<Tweet*>( view->indexWidget(indexFromItem(item(i))) );
}

Tweet* TweetModel::getTweetFromIndex( QModelIndex i )
{
  return dynamic_cast<Tweet*>( view->indexWidget(i) );
}

void TweetModel::emitOpenBrowser( QString address )
{
  emit openBrowser( QUrl( address ) );
}


/*! \class TweetModel
  \brief A class for managing the Tweet list content.

  This class contains a model for a list view displaying status updates.
  It is responsible for behind the scenes management of all the actions
  like adding, sorting, deleting and updating Tweets. Furthermore since
  it is directly connected with all the Tweets, it manages their selecting
  and deselecting.
*/

/*! \fn TweetModel::TweetModel( int margin, StatusList *parentListView, QObject *parent = 0 )
    Creates a tweet list model with a given \a parent.
    \param margin Holds the width of the Tweet list's scrollbar useful when setting
                  size of the Tweet widgets.
    \param parentListView The list view that the model serves for.
    \param parent A parent for the tweet list model.
*/

/*! \fn void TweetModel::deselectCurrentIndex();
    Removes selection from currently highlighted item.
    \sa selectTweet()
*/

/*! \fn void TweetModel::setTheme( const ThemeData &theme )
    Sets \a theme to be the current theme for all the Tweets.
    \param theme The theme to be set.
*/

/*! \fn void TweetModel::setMaxTweetCount( int count )
    Sets maximum amount of Tweets on a list.
    \param count The given maximum Tweets amount.
*/

/*! \fn void TweetModel::insertTweet( Entry *entry )
    Creates a Tweet class instance as a representation of \a entry and adds it
    to the list in an appropriate place (sorting chronogically).
    \param entry The entry on which the new Tweet bases.
    \sa deleteTweet()
*/

/*! \fn void TweetModel::deleteTweet( int id )
    Removes Tweet of the given id from the model and deletes it.
    \param id An id of the Tweet to be deleted.
    \sa insertTweet()
*/

/*! \fn void TweetModel::slotDirectMessagesChanged( bool isEnabled )
    Removes all direct messages from the model and deletes them. Used when
    User disables direct messages download.
*/

/*! \fn void TweetModel::selectTweet( const QModelIndex &index )
    Highlights a Tweet of a given \a index. Used by a mouse press event on the
    Tweet list view.
    \param index The model index of the element to be selected.
    \sa deselectCurrentIndex()
*/

/*! \fn void TweetModel::selectTweet( Tweet *tweet )
    Highlights a given Tweet. Used by a mouse press event on the Tweet's
    internal status display widget.
    \param tweet A Tweet to be selected.
    \sa deselectCurrentIndex()
*/

/*! \fn void TweetModel::markAllAsRead()
    Sets all Tweets' state to Tweet::Read.
*/

/*! \fn void TweetModel::sendNewsInfo()
    Counts unread Tweets and messages and emits newTweets() signal
    to notify MainWindow about a tray icon message to pop up.
*/

/*! \fn void TweetModel::retranslateUi()
    Retranslates all Tweets.
*/

/*! \fn void TweetModel::resizeData( int width, int oldWidth )
    Resizes Tweets according to given values.
    \param width New width of MainWindow.
    \param oldWidth Old width of MainWindow.
*/

/*! \fn void TweetModel::moveFocus( bool up )
    Selects the current Tweet's neighbour, according to the given \a up parameter.
    \param up Selects upper Tweet if true, otherwise selects lower one.
    \sa selectTweet()
    \sa deselectCurrentIndex()
*/

/*! \fn void TweetModel::setImageForUrl( const QString& url, QImage image )
    Assigns the given \a image to all the Tweets having \a url as their profile image URL.
    \param url Profile image URL of the Tweet.
    \param image Image to be set for the Tweet(s).
*/

/*! \fn void TweetModel::setModelToBeCleared( bool wantsPublic, bool userChanged )
    Evaluates an internal flag that indicates if a model has to be completely
    cleared before inserting new Tweets. This occurs e.g. when switching from
    public to friends timeline, or when synchronising with friends timeline
    and changing authenticating user.
    \param wantsPublic Indicates if public timeline will be requested
                       upon next update.
    \param userChanged Indicates if the authentcating user has changed since the
                       last update.
*/

/*! \fn void TweetModel::setPublicTimelineRequested( bool b )
    Sets the flag indicating if the public timeline will be requested upon the
    next connection.
    \param b The new flag's value.
*/

/*! \fn void TweetModel::retweet( QString message )
    Passes the retweet message from a particular Tweet to the MainWindow.
    \param message Retweet status message.
    \sa reply()
*/

/*! \fn void TweetModel::destroy( int id )
    A request to destroy a Tweet, passed from a specific Tweet to the Core class instance.
    \param id The Tweet's id.
*/

/*! \fn void TweetModel::newTweets( int statusesCount, QStringList statusesNames, int messagesCount, QStringList messagesNames )
    Emitted by \ref sendNewsInfo() to notify MainWindow of a new statuses.
    \param statusesCount Amount of the new statuses.
    \param statusesNames List of new statuses authors.
    \param messagesCount Amount of the new direct messages.
    \param messagesNames List of new direct messages authors.
*/

/*! \fn void TweetModel::openBrowser( QUrl address )
    Emitted to pass the request to open web browser to the Core class instance.
    \param address Web address to be accessed.
*/

/*! \fn void TweetModel::reply( const QString &name, int inReplyTo )
    Passes the reply request from a particular Tweet to the MainWindow.
    \param name Login of the original message author.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa retweet()
*/

/*! \fn void TweetModel::about()
    Passes the request to popup an about dialog to the MainWindow.
*/
