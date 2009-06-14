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

#include <QMainWindow>
#include <QUrl>
#include <QList>
#include <QSystemTrayIcon>
#include <twitterapi/twitterapi_global.h>
#include "ui_mainwindow.h"

class QMovie;
class StatusModel;
class Account;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow( QWidget *parent = 0 );
  virtual ~MainWindow();

  StatusListView* getListView();
  int getScrollBarWidth();

  
public slots:
  void setupAccounts( const QList<Account> &accounts, int isPublicTimelineRequested );
  void changeListBackgroundColor( const QColor &newColor );
  void popupMessage( QString message );
  void popupError( const QString &message );
  void retranslateUi();
  void resetStatusEdit();
  void pauseIcon();
  void showProgressIcon();
  void show();
  void about();
  void setListViewModel( StatusModel *model );
  void replaceUrl( const QString &url );

  void statusReplyAction();
  void statusRetweetAction();
  void statusCopylinkAction();
  void statusDeleteAction();
  void statusMarkallasreadAction();
  void statusGototwitterpageAction();
  void statusGotohomepageAction();

signals:
  void updateStatuses();
  void openTwitPicDialog();
  void post( TwitterAPI::SocialNetwork network, const QString &login, QString status, quint64 inReplyTo );
  void openBrowser( QUrl address );
  void settingsDialogRequested();
  void addReplyString( const QString& user, quint64 inReplyTo );
  void addRetweetString( QString message );
  void resizeView( int width, int oldWidth );
  void switchModel( TwitterAPI::SocialNetwork network, const QString &login );
  void switchToPublicTimelineModel( TwitterAPI::SocialNetwork network );
  void shortenUrl( const QString &url );
  void iconStopped();

  void statusMarkeverythingasreadAction();

protected:
  void resizeEvent( QResizeEvent *event );
  void closeEvent( QCloseEvent *event );
  void keyPressEvent ( QKeyEvent *event );
  Ui::MainWindow ui;

private slots:
  void iconActivated( QSystemTrayIcon::ActivationReason reason );
  void emitOpenBrowser( QString address );
  void checkForUpdates();
  void silentCheckForUpdates();
  void readUpdateReply( bool available, const QString &version );
  void silentReadUpdateReply( bool available, const QString &version );
  void changeLabel();
  void sendStatus();
  void resetStatus();
  void configSaveCurrentModel( int index, bool unconditionally = false );
  void selectNextAccount();
  void selectPrevAccount();

private:
  void createConnections();
  void createButtonMenu();
  void createTrayIcon();
#ifdef Q_WS_HILDON
  void createHildonMenu();
#endif

  bool resetUiWhenFinished;

  QMenu *buttonMenu;
  QAction *newstatusAction;
  QAction *newtwitpicAction;
  QAction *gototwitterAction;
  QAction *gotoidenticaAction;
  QAction *gototwitpicAction;
  QAction *checkforupdatesAction;
  QAction *aboutAction;
  QAction *quitAction;
  QMovie *progressIcon;

  QTimer *timer;

  QSystemTrayIcon *trayIcon;
};

#endif //MAINWINDOW_H
