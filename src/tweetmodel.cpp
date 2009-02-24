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

TweetModel::TweetModel( int margin, StatusList *parentListView, QObject *parent ) :
  QStandardItemModel( 0, 0, parent ),
  modelToBeCleared( false ),
  statusesFinished( false ),
  messagesFinished( false ),
  newStatuses( 0 ),
  newMessages( 0 ),
  scrollBarMargin( margin ),
  currentIndex( QModelIndex() ),
  view( parentListView )
{
  connect( view, SIGNAL(clicked(QModelIndex)), this, SLOT(select(QModelIndex)) );
  connect( view, SIGNAL(moveFocus(bool)), this, SLOT(moveFocus(bool)) );
  connect( this, SIGNAL(newTimelineInfo()), SLOT(sendTimelineInfo()) );
  Tweet::setTweetListModel( this );
}

void TweetModel::sendTimelineInfo()
{
  emit newTweets( newStatuses, incomingStatuses, newMessages, incomingMessages );
  newStatuses = newMessages = 0;
  incomingStatuses.clear();
  incomingMessages.clear();
}

void TweetModel::insertTweet( Entry *entry )
{
  if ( modelToBeCleared ) {
    clear();
    modelToBeCleared = false;
  }

  for ( int i = 0; i < rowCount(); ++i ) {
    if ( entry->id() == item(i)->data().value<Entry>().id() ) {
      qDebug() << "found existing entry of the same id";
      if ( !getTweetFromIndex( i )->isRead() ) {
        addUnreadEntry( entry );
      }
      return;
    }
  }
  addUnreadEntry( entry );
  QVariant data = qVariantFromValue( *entry );
  QStandardItem *newItem = new QStandardItem();
  newItem->setData( data );
  Tweet *newTweet;
  if ( entry->getType() == Entry::DirectMessage ) {
    newTweet = new Tweet( *entry, QImage( ":/icons/mail_48.png" ), dynamic_cast<MainWindow*>( this->parent()) );
  } else {
    newTweet = new Tweet( *entry, QImage(), dynamic_cast<MainWindow*>( this->parent()) );
  }
  newTweet->resize( view->width() - scrollBarMargin, newTweet->size().height() );
  newItem->setSizeHint( newTweet->size() );

  if ( rowCount() == 0 ) {
    QStandardItemModel::appendRow( newItem );
    view->setIndexWidget( indexFromItem( newItem ), newTweet );
    return;
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( entry->timestamp() > item(i)->data().value<Entry>().timestamp() ) {
      QStandardItemModel::insertRow( i, newItem );
      view->setIndexWidget( indexFromItem( newItem ), newTweet );
      return;
    }
  }
  QStandardItemModel::appendRow( newItem );
  view->setIndexWidget( indexFromItem( newItem ), newTweet );
}

void TweetModel::deleteTweet( int id )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( id == item(i)->data().value<Entry>().id()  ) {
      removeRow( i );
      return;
    }
  }
}

void TweetModel::addUnreadEntry( Entry *entry )
{
//  if ( !entry->isOwn() ) {
  switch ( entry->getType() ) {
  case Entry::DirectMessage:
    newMessages += 1;
    if ( !incomingMessages.contains( entry->name() ) ) {
      incomingMessages << entry->name();
    }
    break;
  case Entry::Status:
  default:
    newStatuses += 1;
    QString name = entry->isOwn() ? tr( "you" ) : entry->name();
    if ( !incomingStatuses.contains( name ) ) {
      incomingStatuses << name;
    }
  }
  //  }
}

void TweetModel::setImageForUrl( const QString& url, QImage image )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = getTweetFromIndex( i );
    if ( url == item(i)->data().value<Entry>().image() ) {
      aTweet->setIcon( image );
    }
  }
}

void TweetModel::setScrollBarMargin( int width )
{
  scrollBarMargin = width;
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

void TweetModel::setModelToBeCleared( bool publicTimelineRequested, bool userChanged )
{
  deselectCurrent();
  bool timelineChanged = (!publicTimeline && publicTimelineRequested) || (publicTimeline && !publicTimelineRequested);
  if ( (!publicTimeline && !timelineChanged && !userChanged) || (publicTimeline && !timelineChanged) ) {
    qDebug() << publicTimeline << publicTimelineRequested << userChanged << "won't clear list";
    modelToBeCleared = false;
    publicTimeline = publicTimelineRequested;
    return;
  }
  qDebug() << publicTimeline << publicTimelineRequested << userChanged << "will clear list";
  modelToBeCleared = true;
  publicTimeline = publicTimelineRequested;
}

void TweetModel::setPublicTimeline( bool b )
{
  publicTimeline = b;
}

void TweetModel::setTheme( const ThemeData &newTheme )
{
  Tweet::setTheme( newTheme );
  if ( rowCount() > 0 ) {
    for ( int i = 0; i < rowCount(); i++ ) {
      Tweet *aTweet = getTweetFromIndex( i );
      aTweet->applyTheme( aTweet->isRead() ? Settings::Read : Settings::Unread );
    }
  }
}

void TweetModel::select( const QModelIndex &index )
{
  Tweet *aTweet;
  if ( currentIndex != QModelIndex() ) {
    aTweet = getTweetFromIndex( currentIndex );
    aTweet->setRead();
  }
  currentIndex = index;
  aTweet = getTweetFromIndex( currentIndex );
  aTweet->markAsRead();
  aTweet->setActive();
}

void TweetModel::select( Tweet *tweet )
{
  Tweet *aTweet;
  if ( currentIndex != QModelIndex() ) {
    aTweet = getTweetFromIndex( currentIndex );
    aTweet->setRead();
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *matchTweet = getTweetFromIndex( i );
    if ( matchTweet == tweet ) {
      currentIndex = indexFromItem( item(i) );
      break;
    }
  }
  tweet->markAsRead();
  tweet->setActive();
}

void TweetModel::deselectCurrent()
{
  if ( currentIndex != QModelIndex() ) {
    Tweet *aTweet = getTweetFromIndex( currentIndex );
    if ( aTweet->isRead() ) {
      aTweet->markAsRead();
      aTweet->setRead();
    } else {
      aTweet->markAsUnread();
    }
    currentIndex = QModelIndex();
  }
}

void TweetModel::markAllAsRead()
{
  if ( rowCount() > 0 ) {
    for ( int i = 0; i < rowCount(); i++ ) {
      Tweet *aTweet = getTweetFromIndex( i );
      aTweet->markAsRead();
      aTweet->setRead();
    }
  }
}

void TweetModel::moveFocus( bool up )
{
  if ( !rowCount() )
    return;
  if ( currentIndex == QModelIndex() ) {
    currentIndex = this->createIndex( 0, 0 );
  }
  if ( up ) {
    if ( currentIndex.row() > 0 ) {
      select( currentIndex.sibling( currentIndex.row() - 1, 0 ) );
    }
  } else {
    if ( currentIndex.row() < rowCount() - 1 ) {
      select( currentIndex.sibling( currentIndex.row() + 1, 0 ) );
    }
  }
}

Tweet* TweetModel::getTweetFromIndex( int i )
{
  return dynamic_cast<Tweet*>( view->indexWidget(indexFromItem(item(i))) );
}

Tweet* TweetModel::getTweetFromIndex( QModelIndex i )
{
  return dynamic_cast<Tweet*>( view->indexWidget(i) );
}

void TweetModel::retranslateUi()
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = getTweetFromIndex( i );
    aTweet->retranslateUi();
  }
}
