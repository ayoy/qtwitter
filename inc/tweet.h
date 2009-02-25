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

class Tweet : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY( Tweet )

public:
  explicit Tweet( const Entry &entry, const QImage &icon, QWidget *parent );
  virtual ~Tweet();
  void resize( const QSize& );
  void resize( int w, int h );
  void setIcon( const QImage& );
  void retranslateUi();
  QString getUrlForIcon() const;
  bool isRead() const;
  void setRead();

  static ThemeData getTheme();
  static void setTheme( const ThemeData &theme );
  static void setTweetListModel( TweetModel *tweetModel );

  void applyTheme( Settings::ThemeVariant variant = Settings::Unread );

public slots:
  void adjustSize();
  void menuRequested();
  void sendReply();
  void markAsRead();
  void markAsUnread();
  void setActive();
  void retweet();
  void copyLink();

private slots:
  void focusRequest();

signals:
  void reply( const QString& );
  void markAllAsRead();
  void selectMe( Tweet* );
  void postRetweet( const QByteArray & );

protected:
  virtual void changeEvent( QEvent *e );
  void enterEvent( QEvent *e );
  void leaveEvent( QEvent *e );

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
  Entry model;
  QSignalMapper *signalMapper;
  QFont *menuFont;
  bool read;
  static ThemeData currentTheme;
  static TweetModel* tweetListModel;
  Ui::Tweet *m_ui;
};

#endif // TWEET_H
