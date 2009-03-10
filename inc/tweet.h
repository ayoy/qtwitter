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


#ifndef TWEET_H
#define TWEET_H

#include <QtGui/QWidget>
#include <QMenu>
#include <QSignalMapper>
#include <QStandardItem>
#include "mainwindow.h"
#include "core.h"
#include "entry.h"
#include "tweetmodel.h"
#include "settings.h"

namespace Ui {
    class Tweet;
}

/*!
  \brief A widget representation of an Entry class.

  This widget class contains the status data displayed in a custom widget that is put
  on the status list view.
*/
class Tweet : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY( Tweet )

public:

      /*!
    \brief A Tweet's state.

    Used to specify the Tweet's current state, based on the User's selection.
  */
  enum State {
    Unread, /*!< The Tweet is unread. */
    Read, /*!< The Tweet is read. */
    Active /*!< The Tweet is active, i.e. currently highlighted. */
  };

  /*!
    Creates a new Tweet with a given \a parent, fills its data with the given
    \a entry and sets its user image to \a image.
  */
  explicit Tweet( const Entry &entry, const QImage &image, QWidget *parent = 0 );

  /*!
    A virtual destructor.
  */
  virtual ~Tweet();

  /*!
    Resizes a Tweet to the given \a size.
    \param size New size of the Tweet.
    \sa adjustSize()
  */
  void resize( const QSize& size );

  /*!
    Resizes a Tweet to the width given by \a w and height given by \a h.
    Invokes adjustSize() to additionally correct the height of the Tweet.
    \param w New width of the Tweet.
    \param h New height of the Tweet.
    \sa adjustSize()
  */
  void resize( int w, int h );

  /*!
    Sets the Tweet's User profile image to \a image.
    \param image An image to be set.
  */
  void setIcon( const QImage &image );

  /*!
    Applies a theme to the Tweet, according to its current state.
    \sa setState(), getState()
  */
  void applyTheme();

  /*!
    Retranslates all the translatable GUI elements of the Tweet.
    Used when changing UI language on the fly.
  */
  void retranslateUi();

  /*!
    Used to figure out if the Tweet is already read.
    \returns True when Tweet's state is \ref Read or \ref Active, false when Tweet's
             state is \ref Unread.
    \sa getState()
  */
  bool isRead() const;

  /*!
    Used to figure out the actual state of the Tweet.
    \returns The current state of the Tweet.
    \sa setState(), isRead()
  */
  State getState() const;

  /*!
    Sets Tweet's current state to \a state.
    \param state A \ref State to set for the Tweet.
  */
  void setState( Tweet::State state );

  /*!
    Provides information about the theme that is currently set to all the Tweets.
    \returns Current Tweets' theme.
    \sa setTheme()
  */
  static ThemeData getTheme();

  /*!
    Sets the current theme for all the Tweets.
    \param theme A theme to be set.
    \sa getTheme()
  */
  static void setTheme( const ThemeData &theme );

  /*!
    Sets a Tweet list model. The model is used by a Tweet to receive signals
    from a Tweet menu.
    \param tweetModel The model to be set.
  */
  static void setTweetListModel( TweetModel *tweetModel );

public slots:
  /*!
    Adjusts height of the Tweet widget to fit the whole status.
    \sa resize()
  */
  void adjustSize();

  /*!
    Opens a Tweet menu.
  */
  void menuRequested();

  /*!
    Prepares and emits a \ref reply() signal.
    \sa reply()
  */
  void sendReply();

  /*!
    Prepares a retweet (citation of other user's status) and emits a
    \ref retweet() signal to post it.
    \sa retweet()
  */
  void sendRetweet();

  /*!
    Copies a link to the Tweet to system clipboard.
  */
  void copyLink();

signals:
  /*!
    Emitted to notify the MainWindow class instance about the User's request
    to send a reply.
    \param name Login of the User to whom a reply is addressed.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa sendReply()
  */
  void reply( const QString &name, int inReplyTo );

  /*!
    Emitted to notify the Core class instance about the User's request
    to retweet a status.
    \param message A retweet status message prepared by \ref sendRetweet().
    \sa sendRetweet()
  */
  void retweet( QString message );

  /*!
    Emitted to notify the TweetModel class instance about User's request
    to mark all the Tweets in a list as read.
  */
  void markAllAsRead();

  /*!
    Emitted to notify the TweetModel class instance about User's request
    to select (highlight) the current Tweet.
    \param tweet A Tweet to be highlighted. Usually \a this.
  */
  void selectMe( Tweet *tweet );

protected:
  /*!
    Reimplemented to force \ref retranslateUi() upon a language change.
    \param e A QEvent event's representation.
    \sa retranslateUi()
  */
  void changeEvent( QEvent *e );

  /*!
    Reimplemented to show the menu icon when hovered by mouse.
    \param e A QEvent event's representation.
  */
  void enterEvent( QEvent *e );

  /*!
    Reimplemented to hide the menu icon when mouse leaves the Tweet.
    \param e A QEvent event's representation.
  */
  void leaveEvent( QEvent *e );

private slots:
  void focusRequest();

private:
  QMenu *menu;
  QAction *replyAction;
  QAction *retweetAction;
  QAction *copylinkAction;
  QAction *markallasreadAction;
  QAction *gotohomepageAction;
  QAction *gototwitterpageAction;
  QAction *deleteAction;
  QAction *aboutAction;
  State tweetState;
  Entry tweetData;
  QSignalMapper *signalMapper;
  QFont *menuFont;
  static ThemeData currentTheme;
  static TweetModel* tweetListModel;
  Ui::Tweet *m_ui;
};

#endif // TWEET_H
