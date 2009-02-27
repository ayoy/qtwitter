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
#include "statuslist.h"

class Tweet;

class TweetModel : public QStandardItemModel
{
  Q_OBJECT

public:
  TweetModel( int margin, StatusList *parentListView, QObject *parent = 0 );
  void deselectCurrentIndex();
  void setScrollBarMargin( int width );
  void setTheme( const ThemeData &newTheme );

public slots:
  void insertTweet( Entry *entry );
  void deleteTweet( int id );
  void removeDirectMessages();
  void selectTweet( const QModelIndex &index );
  void selectTweet( Tweet *tweet );
  void markAllAsRead();
  void sendNewsInfo();
  void retranslateUi();
  void resizeData( int width, int oldWidth );
  void moveFocus( bool up );
  void setImageForUrl( const QString& url, QImage image );
  void setModelToBeCleared( bool publicTimelineRequested, bool userChanged );
  void setPublicTimelineRequested( bool );

signals:
  void retweet( const QByteArray &status );
  void destroy( int );
  void newTweets( int statusesCount, QStringList statusesNames, int messagesCount, QStringList messagesNames );
  void newTimelineInfo();
  void openBrowser( QString address = QString() );
  void reply( const QString& );
  void about();

private:
  void countUnreadEntries();
  void addUnreadEntry( Entry );
  Tweet* getTweetFromIndex( int );
  Tweet* getTweetFromIndex( QModelIndex );
  bool publicTimeline;
  bool publicTimelineRequested;
  bool modelToBeCleared;
  int newStatusesCount;
  int newMessagesCount;
  QStringList newStatusesNames;
  QStringList newMessagesNames;
  int scrollBarMargin;
  QModelIndex currentIndex;
  StatusList *view;
};

#endif // TWEETMODEL_H
