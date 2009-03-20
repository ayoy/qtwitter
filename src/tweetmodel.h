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
#include "settings.h"
#include "statuslist.h"

class Tweet;

/*!
  \brief A class for managing the Tweet list content.

  This class contains a model for a list view displaying status updates.
  It is responsible for behind the scenes management of all the actions
  like adding, sorting, deleting and updating Tweets. Furthermore since
  it is directly connected with all the Tweets, it manages their selecting
  and deselecting.
*/
class TweetModel : public QStandardItemModel
{
  Q_OBJECT

public:

  /*!
    Creates a tweet list model with a given \a parent.
    \param margin Holds the width of the Tweet list's scrollbar useful when setting
                  size of the Tweet widgets.
    \param parentListView The list view that the model serves for.
  */
  TweetModel( int margin, StatusList *parentListView, QObject *parent = 0 );

  /*!
    Removes selection from currently highlighted item.
    \sa selectTweet()
  */
  void deselectCurrentIndex();

  /*!
    Sets \a theme to be the current theme for all the Tweets.
    \param theme The theme to be set.
  */
  void setTheme( const ThemeData &theme );

  /*!
    Sets maximum amount of Tweets on a list.
    \param count The given maximum Tweets amount.
  */
  void setMaxTweetCount( int count );

public slots:
  /*!
    Creates a Tweet class instance as a representation of \a entry and adds it
    to the list in an appropriate place (sorting chronogically).
    \param entry The entry on which the new Tweet bases.
    \sa deleteTweet()
  */
  void insertTweet( Entry *entry );

  /*!
    Removes Tweet of the given id from the model and deletes it.
    \param id An id of the Tweet to be deleted.
    \sa insertTweet()
  */
  void deleteTweet( int id );

  /*!
    Removes all direct messages from the model and deletes them. Used when
    User disables direct messages download.
  */
  void slotDirectMessagesChanged( bool isEnabled );

  /*!
    Highlights a Tweet of a given \a index. Used by a mouse press event on the
    Tweet list view.
    \param index The model index of the element to be selected.
    \sa deselectCurrentIndex()
  */
  void selectTweet( const QModelIndex &index );

  /*!
    Highlights a given Tweet. Used by a mouse press event on the Tweet's
    internal status display widget.
    \param tweet A Tweet to be selected.
    \sa deselectCurrentIndex()
  */
  void selectTweet( Tweet *tweet );

  /*!
    Sets all Tweets' state to Tweet::Read.
  */
  void markAllAsRead();

  /*!
    Counts unread Tweets and messages and emits newTweets() signal
    to notify MainWindow about a tray icon message to pop up.
  */
  void sendNewsInfo();

  /*!
    Retranslates all Tweets.
  */
  void retranslateUi();

  /*!
    Resizes Tweets according to given values.
    \param width New width of MainWindow.
    \param oldWidth Old width of MainWindow.
  */
  void resizeData( int width, int oldWidth );

  /*!
    Selects the current Tweet's neighbour, according to the given \a up parameter.
    \param up Selects upper Tweet if true, otherwise selects lower one.
    \sa selectTweet()
    \sa deselectCurrentIndex()
  */
  void moveFocus( bool up );

  /*!
    Assigns the given \a image to all the Tweets having \a url as their profile image URL.
    \param url Profile image URL of the Tweet.
    \param image Image to be set for the Tweet(s).
  */
  void setImageForUrl( const QString& url, QImage image );

  /*!
    Evaluates an internal flag that indicates if a model has to be completely
    cleared before inserting new Tweets. This occurs e.g. when switching from
    public to friends timeline, or when synchronising with friends timeline
    and changing authenticating user.
    \param wantsPublic Indicates if public timeline will be requested
                       upon next update.
    \param userChanged Indicates if the authentcating user has changed since the
                       last update.
  */
  void setModelToBeCleared( bool wantsPublic, bool userChanged );

  /*!
    Sets the flag indicating if the public timeline will be requested upon the
    next connection.
    \param b The new flag's value.
  */
  void setPublicTimelineRequested( bool b );

signals:
  /*!
    Passes the retweet message from a particular Tweet to the MainWindow.
    \param message Retweet status message.
    \sa reply()
  */
  void retweet( QString message );

  /*!
    A request to destroy a Tweet, passed from a specific Tweet to the Core class instance.
    \param id The Tweet's id.
  */
  void destroy( int id );

  /*!
    Emitted by \ref sendNewsInfo() to notify MainWindow of a new statuses.
    \param statusesCount Amount of the new statuses.
    \param statusesNames List of new statuses authors.
    \param messagesCount Amount of the new direct messages.
    \param messagesNames List of new direct messages authors.
  */
  void newTweets( int statusesCount, QStringList statusesNames, int messagesCount, QStringList messagesNames );

  /*!
    Emitted to pass the request to open web browser to the Core class instance.
    \param address Web address to be accessed.
  */
  void openBrowser( QUrl address );

  /*!
    Passes the reply request from a particular Tweet to the MainWindow.
    \param name Login of the original message author.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa retweet()
  */
  void reply( const QString &name, int inReplyTo );

  /*!
     Passes the request to popup an about dialog to the MainWindow.
  */
  void about();

private slots:
  void emitOpenBrowser( QString address );

private:
  void countUnreadEntries();
  void addUnreadEntry( Entry );
  bool stripRedundantTweets();
  Tweet* getTweetFromIndex( int );
  Tweet* getTweetFromIndex( QModelIndex );
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
