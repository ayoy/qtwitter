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


#ifndef TWEETMODEL_H
#define TWEETMODEL_H

#include <QStandardItemModel>
#include <QListView>
#include <QUrl>
#include <QPixmap>
#include <QPointer>
#include <twitterapi/twitterapi.h>
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

  Tweet* currentTweet();
  void setLogin( const QString &login );
  const QString& getLogin() const;
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
  void setImageForUrl( const QString& url, QPixmap *image );

signals:
  void retweet( QString message );
  void destroy( const QString &login, int id );
  void newTweets( const QString &login, bool exists );
  void openBrowser( QUrl address );
  void reply( const QString &name, int inReplyTo );
  void about();

private slots:
  void emitOpenBrowser( QString address );

private:
  bool stripRedundantTweets();
  QString login;
  QList<Status> statuses;
  bool isVisible;
  int maxTweetCount;
  int scrollBarMargin;
  QModelIndex currentIndex;
  StatusList *view;
};

struct Status {
  Entry entry;
  TweetModel::TweetState state;
  QPointer<Tweet> tweet;
  QPixmap image;
};

Q_DECLARE_METATYPE(Status)

#endif // TWEETMODEL_H
