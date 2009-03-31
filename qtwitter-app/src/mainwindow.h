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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSystemTrayIcon>
#include <QUrl>
#include "ui_mainwindow.h"
#include "tweetmodel.h"

class QMovie;
class TweetModel;

class MainWindow : public QWidget
{
  Q_OBJECT

  static const QString APP_VERSION;

public:
  MainWindow( QWidget *parent = 0 );
  ~MainWindow();

  StatusList* getListView();
  int getScrollBarWidth();
  void setListViewModel( TweetModel *model );
  
public slots:
  void changeListBackgroundColor( const QColor &newColor );
  void popupMessage( int statusesCount, QStringList namesForStatuses, int messagesCount, QStringList namesForMessages );
  void popupError( const QString &message );
  void retranslateUi();
  void resetStatusEdit();
  void showProgressIcon();
  void about();
  void replaceUrl( const QString &url );

signals:
  void updateTweets();
  void openTwitPicDialog();
  void post( const QByteArray& status, int inReplyTo = -1 );
  void openBrowser( QUrl address );
  void settingsDialogRequested();
  void addReplyString( const QString& user, int inReplyTo );
  void addRetweetString( QString message );
  void resizeView( int width, int oldWidth );
  void shortenUrl( const QString &url );

protected:
  void closeEvent( QCloseEvent *e );

private slots:
  void iconActivated( QSystemTrayIcon::ActivationReason reason );
  void emitOpenBrowser( QString address );
  void changeLabel();
  void sendStatus();
  void resetStatus();

private:
  void resizeEvent( QResizeEvent* );
  bool resetUiWhenFinished;
  QMenu *trayMenu;
  QMenu *buttonMenu;
  QAction *newtweetAction;
  QAction *newtwitpicAction;
  QAction *gototwitterAction;
  QAction *gototwitpicAction;
  QMovie *progressIcon;
  QSystemTrayIcon *trayIcon;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
