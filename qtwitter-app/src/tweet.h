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


#ifndef TWEET_H
#define TWEET_H

#include <QtGui/QWidget>
#include "tweetmodel.h"

class QMenu;
class QSignalMapper;
class TweetModel;
class Entry;
class ThemeData;
class Status;

namespace Ui {
    class Tweet;
}

class Tweet : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY( Tweet )

public:

  explicit Tweet( TweetModel *parentModel, QWidget *parent = 0 );
  explicit Tweet( Entry *entry, TweetModel::TweetState *state, const QPixmap &image, TweetModel *parentModel, QWidget *parent = 0 );
  virtual ~Tweet();

  const Entry& data() const;
  void resize( const QSize& size );
  void resize( int w, int h );

  void initialize();
  void setTweetData( const Status &status );
  void setImage( const QPixmap &pixmap );
  void setState( TweetModel::TweetState state );
  TweetModel::TweetState getState() const;

  static ThemeData getTheme();
  static void setTheme( const ThemeData &theme );

  void applyTheme();
  void retranslateUi();

  int getId() const;

  static void setScrollBarWidth( int width );
  static void setCurrentWidth( int width );
  static void setCurrentLogin( const QString &login );
  static void setCurrentNetwork( TwitterAPI::SocialNetwork network );

public slots:
  void adjustSize();
  void slotReply();
  void slotRetweet();
  void slotCopyLink();
  void slotDelete();

signals:
  void reply( const QString &name, int inReplyTo );
  void retweet( QString message );
  void markAllAsRead();
  void selectMe( Tweet *tweet );
  void deleteStatus( int id );

protected:
  void changeEvent( QEvent *e );
  void enterEvent( QEvent *e );
  void leaveEvent( QEvent *e );
  void mousePressEvent( QMouseEvent *e );

private slots:
  void focusRequest();

private:
  void createMenu();
  void setupMenu();
  QMenu *menu;
  QAction *replyAction;
  QAction *retweetAction;
  QAction *copylinkAction;
  QAction *markallasreadAction;
  QAction *gotohomepageAction;
  QAction *gototwitterpageAction;
  QAction *deleteAction;
  TweetModel::TweetState tweetState;
  const Entry *tweetData;
//  QString originalText;
//  Entry::Type type;
//  int id;
  QSignalMapper *signalMapper;
  static int scrollBarWidth;
  static int currentWidth;
  static ThemeData currentTheme;
  static QString currentLogin;
  static TwitterAPI::SocialNetwork currentNetwork;
  TweetModel *tweetListModel;
  Ui::Tweet *m_ui;
};

#endif // TWEET_H
