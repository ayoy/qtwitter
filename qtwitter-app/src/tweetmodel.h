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
#include <QImage>
#include <QPointer>
#include <entry.h>
#include "statuslist.h"

class Tweet;
class ThemeData;
class TweetModel;
struct Status;



class TweetModel : public QStandardItemModel
{
  Q_OBJECT

public:

  enum TweetState {
    STATE_UNREAD,
    STATE_READ,
    STATE_ACTIVE
  };

  TweetModel( const QString &login, int margin, StatusList *parentListView, QObject *parent = 0 );
  ~TweetModel();

  void deselectCurrentIndex();
  void setTheme( const ThemeData &theme );
  void setMaxTweetCount( int count );
  void setVisible( bool isVisible );
  void display();
  void clear();

public slots:
  void insertTweet( Entry *entry );
  void deleteTweet( int id );
  void slotDirectMessagesChanged( bool isEnabled );
  void selectTweet( const QModelIndex &index );
  void selectTweet( Tweet *tweet );
  void markAllAsRead();
  void checkForUnread();
  void retranslateUi();
  void resizeData( int width, int oldWidth );
  void moveFocus( bool up );
  void setImageForUrl( const QString& url, QImage *image );

signals:
  void retweet( QString message );
  void destroy( const QString &login, int id );
  void newTweets( const QString &login );
  void openBrowser( QUrl address );
  void reply( const QString &name, int inReplyTo );
  void about();

private slots:
  void emitOpenBrowser( QString address );

private:
  bool stripRedundantTweets();
  Status getTweetFromIndex( int );
  Status getTweetFromIndex( QModelIndex );
  QString login;
  QList<Status> statuses;
  bool isVisible;
  bool unread;
  int maxTweetCount;
  int scrollBarMargin;
  QModelIndex currentIndex;
  StatusList *view;
};

struct Status {
  Entry entry;
  TweetModel::TweetState state;
  QPointer<Tweet> tweet;
  QImage image;
};

Q_DECLARE_METATYPE(Status)

#endif // TWEETMODEL_H
