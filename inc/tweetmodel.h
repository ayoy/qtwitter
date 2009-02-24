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


#ifndef TWEETMODEL_H
#define TWEETMODEL_H

#include <QStandardItemModel>
#include <QListView>
#include "entry.h"
#include "settings.h"

class Tweet;

class TweetModel : public QStandardItemModel
{
  Q_OBJECT

  bool publicTimeline;
  bool modelToBeCleared;
  bool statusesFinished;
  bool messagesFinished;
  QStringList incomingStatuses;
  QStringList incomingMessages;
  int scrollBarMargin;
  QModelIndex currentIndex;
  QListView *view;
  void addUnreadEntry( Entry* );
  Tweet* getTweetFromIndex( int );
  Tweet* getTweetFromIndex( QModelIndex );

public:
  TweetModel( int margin, QListView *parentListView, QObject *parent = 0 );
  void setScrollBarMargin( int width );
  void setTheme( const ThemeData &newTheme );
  void deselectCurrent();

public slots:
  void insertTweet( Entry *entry );
  void deleteTweet( int id );
  void setImageForUrl( const QString& url, QImage image );
  void resizeData( int width, int oldWidth );
  void setModelToBeCleared( bool publicTimelineRequested, bool userChanged );
  void setPublicTimeline( bool );
  void retranslateUi();
  void select( const QModelIndex &index );
  void select( Tweet *tweet );
  void markAllAsRead();
  void sendTimelineInfo();

signals:
  void newTweets( QStringList namesForStatuses, QStringList namesForMessages );
  void newTimelineInfo();
};

#endif // TWEETMODEL_H
