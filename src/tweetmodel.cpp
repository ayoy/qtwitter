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

TweetModel::TweetModel( int margin, QListView *parentListView, QObject *parent ) :
  QStandardItemModel( 0, 0, parent ),
  scrollBarMargin( margin ),
  view( parentListView )
{}

bool TweetModel::insertTweet( Entry *entry )
{
  qDebug() << entry->getIndex() << entry->name();
  QStandardItem *newItem = new QStandardItem();
  newItem->setData( entry->timestamp() );
  Tweet *newTweet;
  qDebug() << entry->getType();
  if ( entry->getType() == Entry::DirectMessage ) {
    newTweet = new Tweet( *entry, QImage( ":/icons/mail_48.png" ), dynamic_cast<MainWindow*>(this->parent()) );
  } else {
    newTweet = new Tweet( *entry, QImage(), dynamic_cast<MainWindow*>(this->parent()) );
  }
  newTweet->resize( view->width() - scrollBarMargin, newTweet->size().height() );
  newItem->setSizeHint( newTweet->size() );

  if ( rowCount() == 0 ) {
    QStandardItemModel::appendRow( newItem );
    view->setIndexWidget( indexFromItem( newItem ), newTweet );
    return true;
  }
  for ( int i = 0; i < rowCount(); i++ ) {
    if ( entry->timestamp() > item(i)->data().toDateTime() ) {
      QStandardItemModel::insertRow( i, newItem );
      view->setIndexWidget( indexFromItem( newItem ), newTweet );
      return true;
    }
  }
  QStandardItemModel::appendRow( newItem );
  view->setIndexWidget( indexFromItem( newItem ), newTweet );
  return true;
}

void TweetModel::deleteTweet( int id )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( view->indexWidget( indexFromItem( item(i) ) ) );
    if ( id == aTweet->getModel()->id() ) {
      removeRow( i );
      return;
    }
  }
}

void TweetModel::setImageForUrl( const QString& url, QImage image )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( view->indexWidget( indexFromItem( item(i) ) ) );
    if ( !aTweet->getUrlForIcon().compare( url ) ) {
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
  QSize itemSize;
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( view->indexWidget( indexFromItem( item(i) ) ) );
    aTweet->resize( width - scrollBarMargin, aTweet->size().height() );
    itemSize = item(i)->sizeHint();
    itemSize.rwidth() += width - oldWidth;
    itemSize.rheight() = aTweet->size().height();
    item(i)->setSizeHint( itemSize );
  }
}
