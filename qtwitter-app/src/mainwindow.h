/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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
#include <QList>
#include "ui_mainwindow.h"
#include "tweetmodel.h"

class QMovie;
class TwitterAccount;

class MainWindow : public QWidget
{
  Q_OBJECT

  static const QString APP_VERSION;

public:
  MainWindow( QWidget *parent = 0 );
  virtual ~MainWindow();

  StatusList* getListView();
  int getScrollBarWidth();
  
public slots:
  void setupTwitterAccounts( const QList<TwitterAccount> &accounts, bool isPublicTimelineRequested );
  void changeListBackgroundColor( const QColor &newColor );
  void popupMessage( QString message );
  void popupError( const QString &message );
  void retranslateUi();
  void resetStatusEdit();
  void showProgressIcon();
  void about();
  void setListViewModel( TweetModel *model );
  void replaceUrl( const QString &url );

signals:
  void setCurrentModel( const QString &login );
  void updateTweets();
  void openTwitPicDialog();
  void post( const QString &login, QString status, int inReplyTo );
  void openBrowser( QUrl address );
  void settingsDialogRequested();
  void addReplyString( const QString& user, int inReplyTo );
  void addRetweetString( QString message );
  void resizeView( int width, int oldWidth );
  void switchModel( const QString &login );
  void switchToPublicTimelineModel();
  void shortenUrl( const QString &url );

protected:
  void closeEvent( QCloseEvent *e );
  void resizeEvent( QResizeEvent* );
  Ui::MainWindow ui;

private slots:
  void iconActivated( QSystemTrayIcon::ActivationReason reason );
  void emitOpenBrowser( QString address );
  void changeLabel();
  void sendStatus();
  void resetStatus();
  void configSaveCurrentModel( int index );
  void selectNextAccount();
  void selectPrevAccount();

private:
  void createConnections();
  void createMenu();
  void createTrayIcon();
  bool resetUiWhenFinished;
  QMenu *trayMenu;
  QMenu *buttonMenu;
  QAction *newtweetAction;
  QAction *newtwitpicAction;
  QAction *gototwitterAction;
  QAction *gototwitpicAction;
  QMovie *progressIcon;
  QSystemTrayIcon *trayIcon;
};

#endif //MAINWINDOW_H
