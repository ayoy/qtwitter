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
  countUnreadEntries();
  emit newTweets( newStatuses, incomingStatuses, newMessages, incomingMessages );
  newStatuses = newMessages = 0;
  publicTimeline = publicTimelineRequested;
  incomingStatuses.clear();
  incomingMessages.clear();
}

void TweetModel::countUnreadEntries()
{
  for ( int i = 0; i < rowCount(); ++i ) {
    if ( !getTweetFromIndex( i )->isRead() ) {
      addUnreadEntry( item(i)->data().value<Entry>() );
    }
  }
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
      return;
    }
  }
  QVariant data = qVariantFromValue( *entry );
  QStandardItem *newItem = new QStandardItem();
  newItem->setData( data );
  Tweet *newTweet;
  if ( entry->getType() == Entry::DirectMessage ) {
    newTweet = new Tweet( *entry, QImage( ":/icons/mail_48.png" ), dynamic_cast<MainWindow*>( parent()) );
  } else {
    newTweet = new Tweet( *entry, QImage(), dynamic_cast<MainWindow*>( parent()) );
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

void TweetModel::addUnreadEntry( Entry entry )
{
  switch ( entry.getType() ) {
  case Entry::DirectMessage:
    newMessages += 1;
    if ( !incomingMessages.contains( entry.name() ) ) {
      incomingMessages << entry.name();
    }
    break;
  case Entry::Status:
  default:
    newStatuses += 1;
    QString name = entry.isOwn() ? tr( "you" ) : entry.name();
    if ( !incomingStatuses.contains( name ) ) {
      incomingStatuses << name;
    }
  }
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
  deselectCurrent();
  modelToBeCleared = true;
  publicTimelineRequested = wantsPublic;
}

void TweetModel::removeDirectMessages()
{
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( item(i)->data().value<Entry>().getType() == Entry::DirectMessage ) {
      removeRow( i );
      i--;
    }
  }
}

void TweetModel::setPublicTimelineRequested( bool b )
{
  publicTimelineRequested = b;
}

void TweetModel::setTheme( const ThemeData &newTheme )
{
  Tweet::setTheme( newTheme );
  if ( rowCount() > 0 ) {
    for ( int i = 0; i < rowCount(); i++ ) {
      Tweet *aTweet = getTweetFromIndex( i );
      aTweet->applyTheme( aTweet->getState() );
    }
  }
}

void TweetModel::select( const QModelIndex &index )
{
  Tweet *aTweet;
  if ( currentIndex != QModelIndex() ) {
    aTweet = getTweetFromIndex( currentIndex );
    aTweet->setState( Tweet::Read );
  }
  currentIndex = index;
  aTweet = getTweetFromIndex( currentIndex );
  aTweet->setState( Tweet::Active );
}

void TweetModel::select( Tweet *tweet )
{
  if ( currentIndex != QModelIndex() ) {
    Tweet *aTweet = getTweetFromIndex( currentIndex );
    aTweet->setState( Tweet::Read );
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *matchTweet = getTweetFromIndex( i );
    if ( matchTweet == tweet ) {
      currentIndex = indexFromItem( item(i) );
      break;
    }
  }
  tweet->setState( Tweet::Active );
}

void TweetModel::deselectCurrent()
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

void TweetModel::markAllAsRead()
{
  if ( rowCount() > 0 ) {
    for ( int i = 0; i < rowCount(); i++ ) {
      Tweet *aTweet = getTweetFromIndex( i );
      aTweet->setState( Tweet::Read );
    }
  }
}

void TweetModel::moveFocus( bool up )
{
  if ( !rowCount() )
    return;
  if ( currentIndex == QModelIndex() ) {
    currentIndex = this->index( 0, 0 );
    select( currentIndex );
    return;
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
