/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSystemTrayIcon>
#include <QUrl>
#include <QList>
#include <twitterapi/twitterapi_global.h>
#include "ui_mainwindow.h"

class QMovie;
class TweetModel;
class Account;

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow( QWidget *parent = 0 );
  virtual ~MainWindow();

  StatusList* getListView();
  int getScrollBarWidth();

  
public slots:
  void setupAccounts( const QList<Account> &accounts, int isPublicTimelineRequested );
  void changeListBackgroundColor( const QColor &newColor );
  void popupMessage( QString message );
  void popupError( const QString &message );
  void retranslateUi();
  void resetStatusEdit();
  void showProgressIcon();
  void about();
  void setListViewModel( TweetModel *model );
  void replaceUrl( const QString &url );

  void tweetReplyAction();
  void tweetRetweetAction();
  void tweetCopylinkAction();
  void tweetDeleteAction();
  void tweetMarkallasreadAction();
  void tweetGototwitterpageAction();
  void tweetGotohomepageAction();

signals:
  void updateTweets();
  void openTwitPicDialog();
  void post( TwitterAPI::SocialNetwork network, const QString &login, QString status, int inReplyTo );
  void openBrowser( QUrl address );
  void settingsDialogRequested();
  void addReplyString( const QString& user, int inReplyTo );
  void addRetweetString( QString message );
  void resizeView( int width, int oldWidth );
  void switchModel( TwitterAPI::SocialNetwork network, const QString &login );
  void switchToPublicTimelineModel( TwitterAPI::SocialNetwork network );
  void shortenUrl( const QString &url );

protected:
  void closeEvent( QCloseEvent *e );
  void resizeEvent( QResizeEvent* );
  void keyPressEvent( QKeyEvent* );
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
  QAction *aboutAction;
  QAction *quitAction;
  QMovie *progressIcon;
  QSystemTrayIcon *trayIcon;
};

#endif //MAINWINDOW_H
