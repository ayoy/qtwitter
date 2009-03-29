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
#include <QUrl>
#include "entry.h"
#include "statuslist.h"
#include <QImage>

class Tweet;
class ThemeData;

struct Status {
  Entry entry;
  Tweet *tweet;
  QImage image;
};

Q_DECLARE_METATYPE(Status)

class TweetModel : public QStandardItemModel
{
  Q_OBJECT

public:
  TweetModel( int margin, StatusList *parentListView, QObject *parent = 0 );
  void deselectCurrentIndex();
  void setTheme( const ThemeData &theme );
  void setMaxTweetCount( int count );

public slots:
  void insertTweet( Entry *entry );
  void deleteTweet( int id );
  void slotDirectMessagesChanged( bool isEnabled );
  void selectTweet( const QModelIndex &index );
  void selectTweet( Tweet *tweet );
  void markAllAsRead();
  void sendNewsInfo();
  void retranslateUi();
  void resizeData( int width, int oldWidth );
  void moveFocus( bool up );
  void setImageForUrl( const QString& url, QImage image );
  void setModelToBeCleared( bool wantsPublic, bool userChanged );
  void setPublicTimelineRequested( bool b );

signals:
  void retweet( QString message );
  void destroy( int id );
  void newTweets( int statusesCount, QStringList statusesNames, int messagesCount, QStringList messagesNames );
  void openBrowser( QUrl address );
  void reply( const QString &name, int inReplyTo );
  void about();

private slots:
  void emitOpenBrowser( QString address );

private:
  void countUnreadEntries();
  void addUnreadEntry( Entry );
  bool stripRedundantTweets();
  Status getTweetFromIndex( int );
  Status getTweetFromIndex( QModelIndex );
  bool isVisible;
  bool publicTimeline;
  bool publicTimelineRequested;
  bool modelToBeCleared;
  int newStatusesCount;
  int newMessagesCount;
  QStringList newStatusesNames;
  QStringList newMessagesNames;
  int maxTweetCount;
  int scrollBarMargin;
  QModelIndex currentIndex;
  StatusList *view;
};

#endif // TWEETMODEL_H
