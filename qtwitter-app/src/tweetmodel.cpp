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
    if ( entry->id() == item(i)->data().value<Entry>().id() ) {
      qDebug() << "found existing entry of the same id";
      return;
    }
  }
  QVariant data = qVariantFromValue( *entry );
  QStandardItem *newItem = new QStandardItem();
  newItem->setData( data );
  Tweet *newTweet;
  if ( entry->getType() == Entry::DirectMessage )
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
    if ( entry->timestamp() > item(i)->data().value<Entry>().timestamp() ) {
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
    if ( id == item(i)->data().value<Entry>().id()  ) {
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
    if ( item(i)->data().value<Entry>().getType() == Entry::DirectMessage ) {
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
    if ( url == item(i)->data().value<Entry>().image() ) {
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
  switch ( entry.getType() ) {
  case Entry::DirectMessage:
    newMessagesCount += 1;
    if ( !newMessagesNames.contains( entry.name() ) ) {
      newMessagesNames << entry.name();
    }
    break;
  case Entry::Status:
  default:
    newStatusesCount += 1;
    QString name = entry.isOwn() ? tr( "you" ) : entry.name();
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
