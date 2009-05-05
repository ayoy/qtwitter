/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include <twitterapi/twitterapi.h>
#include "tweetmodel.h"
#include "tweet.h"
#include "mainwindow.h"
#include "settings.h"

TweetModel::TweetModel( const QString &login, int margin, StatusList *parentListView, QObject *parent ) :
  QStandardItemModel( 0, 0, parent ),
  login( login ),
  isVisible( false ),
  maxTweetCount( 20 ),
  scrollBarMargin( margin ),
  currentIndex( QModelIndex() ),
  view( parentListView )
{}

TweetModel::~TweetModel()
{
  foreach ( Status status, statuses ) {
    if ( !status.tweet.isNull() )
      delete status.tweet;
  }
}

void TweetModel::setLogin( const QString &login )
{
  this->login = login;
}

const QString& TweetModel::getLogin() const
{
  return login;
}

Tweet* TweetModel::currentTweet()
{
  if ( !currentIndex.isValid() )
    return 0;

  return statuses[ currentIndex.row() ].tweet;
}

void TweetModel::deselectCurrentIndex()
{
  if ( currentIndex.isValid() ) {
    Status *status = &statuses[ currentIndex.row() ];
    if ( status->state != TweetModel::STATE_UNREAD )
      status->state = TweetModel::STATE_READ;
    if ( isVisible )
      status->tweet->applyTheme();
    currentIndex = QModelIndex();
  }
}

void TweetModel::setTheme( const ThemeData &theme )
{
  Tweet::setTheme( theme );
  if ( isVisible && rowCount() > 0 ) {
    Tweet *aTweet;
    for ( int i = 0; i < rowCount(); i++ ) {
      aTweet = statuses[i].tweet;
      aTweet->applyTheme();
    }
  }
}

void TweetModel::setMaxTweetCount( int count )
{
  maxTweetCount = count;
  stripRedundantTweets();
}

void TweetModel::setVisible( bool isVisible )
{
  if ( this->isVisible == isVisible )
    return;

  this->isVisible = isVisible;
  if ( this->isVisible ) {
    connect( view, SIGNAL(clicked(QModelIndex)), this, SLOT(selectTweet(QModelIndex)) );
    connect( view, SIGNAL(moveFocus(bool)), this, SLOT(moveFocus(bool)) );
  } else {
    disconnect( view, SIGNAL(clicked(QModelIndex)), this, SLOT(selectTweet(QModelIndex)) );
    disconnect( view, SIGNAL(moveFocus(bool)), this, SLOT(moveFocus(bool)) );
    for (int i = 0; i < rowCount(); i++ ) {
      Q_ASSERT( statuses[i].tweet == view->indexWidget( item(i)->index() ) );
      delete view->indexWidget( item(i)->index() );
      Q_ASSERT( statuses[i].tweet == 0 );
    }
  }
}

void TweetModel::display()
{
  if ( isVisible )
    return;

  view->setUpdatesEnabled( false );
  setVisible( true );
  Status *status;
  for (int i = 0; i < rowCount(); i++ ) {
    status = &statuses[i];
    if ( status->tweet ) {
      delete status->tweet;
    }
    Q_ASSERT( status->tweet == 0 );
    if ( status->entry.type == Entry::DirectMessage )
      status->tweet = new Tweet( &status->entry, &status->state, QPixmap( ":/icons/mail_48.png" ), this );
    else
      status->tweet = new Tweet( &status->entry, &status->state, status->image, this );
    status->tweet->setTweetData( &status->entry, &status->state );
    status->tweet->applyTheme();
    status->tweet->resize( view->width() - scrollBarMargin, status->tweet->height() );
    item(i)->setSizeHint( status->tweet->size() );
    if ( i == rowCount() - 1 )
      resizeData( view->width(), view->width() - 1 );
  }
  for (int i = 0; i < rowCount(); i++ ) {
    view->setIndexWidget( item(i)->index(), statuses[i].tweet );
    Q_ASSERT( statuses[i].tweet == view->indexWidget( item(i)->index() ) );
    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  }
  if ( currentIndex.isValid() ) {
    statuses[ currentIndex.row() ].state = TweetModel::STATE_ACTIVE;
    statuses[ currentIndex.row() ].tweet->applyTheme();
    view->scrollTo( currentIndex );
  }
  view->setUpdatesEnabled( true );
}

void TweetModel::clear()
{
  int rc = rowCount();
  if ( rc == 0 )
    return;

  if ( isVisible )
    for ( int i = rc - 1; i >= 0; i-- ) {
        delete view->indexWidget( item(i)->index() );
    }
  QStandardItemModel::clear();
  statuses.clear();
}

void TweetModel::insertTweet( Entry *entry )
{
  for ( int i = 0; i < rowCount(); ++i ) {
    if ( entry->id == statuses[i].entry.id ) {
      qDebug() << "found existing entry of the same id";
      return;
    }
  }

  Status status;
  status.state = TweetModel::STATE_UNREAD;
  status.entry = *entry;
  if ( status.entry.type == Entry::DirectMessage )
    status.tweet = new Tweet( &status.entry, &status.state, QPixmap( ":/icons/mail_48.png" ), this );
  else
    status.tweet = new Tweet( &status.entry, &status.state, QPixmap(), this );
  status.tweet->setTweetData( &status.entry, &status.state );
  qDebug() << login << " type: " << status.entry.type << " id:" << status.entry.id;

  QStandardItem *newItem = new QStandardItem;
  if ( isVisible ) {
    status.tweet->resize( view->width() - scrollBarMargin, status.tweet->size().height() );
    newItem->setSizeHint( status.tweet->size() );
  }
  if ( rowCount() == 0 ) {
    QStandardItemModel::appendRow( newItem );
    statuses.append( status );
    statuses.last().tweet->setTweetData( &statuses.last().entry, &statuses.last().state );
    if ( isVisible )
      view->setIndexWidget( newItem->index(), status.tweet );
    return;
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( status.entry.timestamp > statuses[i].entry.timestamp ) {
      QStandardItemModel::insertRow( i, newItem );
      statuses.insert( i, status );
      statuses[i].tweet->setTweetData( &statuses[i].entry, &statuses[i].state );
      if ( isVisible )
        view->setIndexWidget( newItem->index(), status.tweet );
      if ( currentIndex.isValid() && currentIndex.row() >= i && currentIndex.row() < (maxTweetCount - 1) )
        selectTweet( currentIndex.sibling( currentIndex.row() + 1, 0 ) );
      stripRedundantTweets();
      return;
    }
  }
  if ( stripRedundantTweets() ) {
    delete status.tweet;
    delete newItem;
    return;
  }
  QStandardItemModel::appendRow( newItem );
  statuses.append( status );
  statuses.last().tweet->setTweetData( &statuses.last().entry, &statuses.last().state );
  if ( isVisible )
    view->setIndexWidget( newItem->index(), status.tweet );
}

void TweetModel::deleteTweet( int id )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( id == statuses[i].entry.id  ) {
      if ( isVisible )
        delete view->indexWidget( item(i)->index() );
      removeRow(i);
      Q_ASSERT(statuses[i].tweet == 0 );
      statuses.removeAt(i);
      return;
    }
  }
}

void TweetModel::slotDirectMessagesChanged( bool isEnabled )
{
  if ( isEnabled )
    return;
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( statuses[i].entry.type == Entry::DirectMessage ) {
      if ( isVisible ) {
        delete view->indexWidget( item(i)->index() );
      } else {
        delete statuses[i].tweet;
      }
      removeRow(i);
      Q_ASSERT(statuses[i].tweet == 0 );
      statuses.removeAt(i);
      i--;
    }
  }
}

void TweetModel::selectTweet( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  Status *status;
  if ( currentIndex.isValid() ) {
    status = &statuses[ currentIndex.row() ];
    if ( status->state != TweetModel::STATE_UNREAD )
      status->state = TweetModel::STATE_READ;
    status->tweet->applyTheme();
  }
  currentIndex = index;
  status = &statuses[ index.row() ];
  status->state = TweetModel::STATE_ACTIVE;
  if ( isVisible ) {
    status->tweet->applyTheme();
    view->setCurrentIndex( currentIndex );
  }
}

void TweetModel::selectTweet( Tweet *tweet )
{
  Status *status;
  if ( currentIndex.isValid() ) {
    status = &statuses[ currentIndex.row() ];
    if ( status->state != TweetModel::STATE_UNREAD )
      status->state = TweetModel::STATE_READ;
    status->tweet->applyTheme();
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    status = &statuses[i];
    if ( status->tweet == tweet ) {
      status->state = TweetModel::STATE_ACTIVE;
      status->tweet->applyTheme();
      currentIndex = item(i)->index();
      if ( isVisible )
        view->setCurrentIndex( currentIndex );
      break;
    }
  }
}

void TweetModel::markAllAsRead()
{
  if ( rowCount() > 0 ) {
    Status *status;
    for ( int i = 0; i < rowCount(); i++ ) {
      status = &statuses[i];
      if ( i == currentIndex.row() )
        status->state = TweetModel::STATE_ACTIVE;
      else
        status->state = TweetModel::STATE_READ;
      status->tweet->applyTheme();
    }
  }
}

void TweetModel::checkForUnread()
{
  qDebug() << "TweetModel::checkForUnread( " + login + " );";
  for ( int i = 0; i < rowCount(); ++i ) {
    if ( statuses[i].state == TweetModel::STATE_UNREAD ) {
      emit newTweets( login, true );
      return;
    }
  }
  emit newTweets( login, false );
}

void TweetModel::retranslateUi()
{
  if ( !isVisible )
    return;
  for ( int i = 0; i < rowCount(); i++ )
    statuses[i].tweet.data()->retranslateUi();
}

void TweetModel::resizeData( int width, int oldWidth )
{
  if ( !isVisible || rowCount() == 0 )
    return;

  QSize itemSize;
  Tweet *aTweet;
  for ( int i = 0; i < rowCount(); i++ ) {
    aTweet = statuses[i].tweet;
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
  if ( !currentIndex.isValid() ) {
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

void TweetModel::setImageForUrl( const QString& url, QPixmap *image )
{
  Status *status;
  for ( int i = 0; i < rowCount(); i++ ) {
    status = &statuses[i];
    if ( url == status->entry.image ) {
      status->image = *image;
      if ( isVisible )
        status->tweet->setIcon( *image );
    }
  }
}

bool TweetModel::stripRedundantTweets()
{
  if ( rowCount() >= maxTweetCount ) {
    if ( currentIndex.row() > maxTweetCount - 1 ) {
      selectTweet( currentIndex.sibling( maxTweetCount - 1, 0 ) );
    }
    int currentRowCount = rowCount();
    for (int i = currentRowCount - 1; i >= maxTweetCount; i-- ) {
      if ( isVisible ) {
        delete view->indexWidget( item(i)->index() );
      } else {
        delete statuses[i].tweet;
      }
      Q_ASSERT(statuses[i].tweet == 0 );
      statuses.removeAt(i);
      QStandardItemModel::removeRow(i);
    }
    return true;
  }
  return false;
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
    Sets all Tweets' state to TweetModel::STATE_READ.
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

/*! \fn void TweetModel::setImageForUrl( const QString& url, QPixmap image )
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
