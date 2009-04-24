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
#include <twitterapi/twitterapi.h>
#include "tweetmodel.h"

class ThemeData;

namespace Ui {
    class Tweet;
}

class Tweet : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY( Tweet )

public:

  explicit Tweet( Entry *entry, TweetModel::TweetState *state, const QImage &image, TweetModel *parentModel, QWidget *parent = 0 );
  virtual ~Tweet();

  const Entry& data() const;
  void resize( const QSize& size );
  void resize( int w, int h );

  void setTweetData( Entry *entry, TweetModel::TweetState *state );
  void setIcon( const QImage &image );
  void applyTheme();
  void retranslateUi();

  bool isRead() const;
  TweetModel::TweetState getState() const;
  void setState( TweetModel::TweetState state );
  static ThemeData getTheme();
  static void setTheme( const ThemeData &theme );

public slots:
  void adjustSize();
  void menuRequested();
  void slotReply();
  void slotRetweet();
  void slotCopyLink();
  void slotDelete();

signals:
  void reply( const QString &name, int inReplyTo );
  void retweet( QString message );
  void markAllAsRead();
  void selectMe( Tweet *tweet );
  void deleteStatus( const QString &login, int id );

protected:
  void changeEvent( QEvent *e );
  void enterEvent( QEvent *e );
  void leaveEvent( QEvent *e );

private slots:
  void focusRequest();

private:
  void createMenu();
  QMenu *menu;
  QAction *replyAction;
  QAction *retweetAction;
  QAction *copylinkAction;
  QAction *markallasreadAction;
  QAction *gotohomepageAction;
  QAction *gototwitterpageAction;
  QAction *deleteAction;
  TweetModel::TweetState *tweetState;
  Entry *tweetData;
  QSignalMapper *signalMapper;
  static ThemeData currentTheme;
  TweetModel *tweetListModel;
  Ui::Tweet *m_ui;
};

#endif // TWEET_H
