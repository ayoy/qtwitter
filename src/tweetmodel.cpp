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
  modelToBeCleared( false ),
  scrollBarMargin( margin ),
  view( parentListView )
{
}

void TweetModel::insertTweet( Entry *entry )
{
  if ( modelToBeCleared ) {
    clear();
    modelToBeCleared = false;
  }
  QVariant data = qVariantFromValue( *entry );
  QStandardItem *newItem = new QStandardItem();
  newItem->setData( data );
  Tweet *newTweet;
  if ( entry->getType() == Entry::DirectMessage ) {
    newTweet = new Tweet( *entry, QImage( ":/icons/mail_48.png" ), currentTheme, dynamic_cast<MainWindow*>( this->parent()) );
  } else {
    newTweet = new Tweet( *entry, QImage(), currentTheme, dynamic_cast<MainWindow*>( this->parent()) );
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
  return;
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

void TweetModel::setImageForUrl( const QString& url, QImage image )
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( view->indexWidget( indexFromItem( item(i) ) ) );
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
    Tweet *aTweet = dynamic_cast<Tweet*>( view->indexWidget( indexFromItem( item(i) ) ) );
    aTweet->resize( width - scrollBarMargin, aTweet->size().height() );
    itemSize = item(i)->sizeHint();
    itemSize.rwidth() += width - oldWidth;
    itemSize.rheight() = aTweet->size().height();
    item(i)->setSizeHint( itemSize );
  }
}

void TweetModel::setModelToBeCleared()
{
  modelToBeCleared = true;
}

void TweetModel::setTheme( const ThemeData &newTheme )
{
  currentTheme = newTheme;
  if ( rowCount() > 0 ) {
    for ( int i = 0; i < rowCount(); i++ ) {
      Tweet *aTweet = dynamic_cast<Tweet*>( view->indexWidget( indexFromItem( item(i) ) ) );
      aTweet->applyTheme( newTheme );
    }
  }
}

void TweetModel::retranslateUi()
{
  for ( int i = 0; i < rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( view->indexWidget( indexFromItem( item(i) ) ) );
    aTweet->retranslateUi();
  }
}

