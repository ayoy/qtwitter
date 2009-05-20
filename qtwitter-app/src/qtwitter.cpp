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

#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QMenuBar>
#include <QSystemTrayIcon>
#include <QSignalMapper>
#include <QSettings>
#include <twitterapi/twitterapi_global.h>
#include "qtwitter.h"
#include "core.h"
#include "mainwindow.h"
#include "twitpicview.h"
#include "tweet.h"
#include "settings.h"
#include "account.h"

extern ConfigFile settings;

Qtwitter::Qtwitter( QWidget *parent, Qt::WindowFlags flags)
  : QMainWindow( parent, flags )
{
  mainwindow = new MainWindow(this);
  setCentralWidget(mainwindow);

  connect( mainwindow, SIGNAL(switchModel(TwitterAPI::SocialNetwork,QString)), SLOT(setCurrentModel(TwitterAPI::SocialNetwork,QString)) );
  connect( mainwindow, SIGNAL(switchToPublicTimelineModel(TwitterAPI::SocialNetwork)), SLOT(setPublicTimelineModel(TwitterAPI::SocialNetwork)) );

  core = new Core( mainwindow );
  connect( mainwindow, SIGNAL(updateTweets()), core, SLOT(forceGet()) );
  connect( mainwindow, SIGNAL(openBrowser(QUrl)), core, SLOT(openBrowser(QUrl)) );
  connect( mainwindow, SIGNAL(post(TwitterAPI::SocialNetwork,QString,QString,int)), core, SLOT(post(TwitterAPI::SocialNetwork,QString,QString,int)) );
  connect( mainwindow, SIGNAL(resizeView(int,int)), core, SIGNAL(resizeData(int,int)));
  connect( mainwindow, SIGNAL(shortenUrl(QString)), core, SLOT(shortenUrl(QString)));
  connect( core, SIGNAL(accountsUpdated(QList<Account>,int)), mainwindow, SLOT(setupAccounts(QList<Account>,int)) );
  connect( core, SIGNAL(urlShortened(QString)), mainwindow, SLOT(replaceUrl(QString)));
  connect( core, SIGNAL(about()), this, SLOT(about()) );
  connect( core, SIGNAL(addReplyString(QString,int)), mainwindow, SIGNAL(addReplyString(QString,int)) );
  connect( core, SIGNAL(addRetweetString(QString)), mainwindow, SIGNAL(addRetweetString(QString)) );
  connect( core, SIGNAL(errorMessage(QString)), mainwindow, SLOT(popupError(QString)) );
  connect( core, SIGNAL(resetUi()), mainwindow, SLOT(resetStatusEdit()) );
  connect( core, SIGNAL(requestStarted()), mainwindow, SLOT(showProgressIcon()) );
  if ( QSystemTrayIcon::supportsMessages() )
    connect( core, SIGNAL(sendNewsReport(QString)), this, SLOT(popupMessage(QString)) );

  twitpic = new TwitPicView( mainwindow );
  connect( twitpic, SIGNAL(uploadPhoto(QString,QString,QString)), core, SLOT(uploadPhoto(QString,QString,QString)) );
  connect( twitpic, SIGNAL(abortUpload()), core, SLOT(abortUploadPhoto()) );
  connect( mainwindow, SIGNAL(openTwitPicDialog()), twitpic, SLOT(show()) );
  connect( core, SIGNAL(twitPicResponseReceived()), twitpic, SLOT(resetForm()) );
  connect( core, SIGNAL(twitPicDataSendProgress(int,int)), twitpic, SLOT(showUploadProgress(int,int)) );
  connect( core, SIGNAL(accountsUpdated(QList<Account>,int)), twitpic, SLOT(setupAccounts(QList<Account>)) );

  settingsDialog = new Settings( mainwindow, core, twitpic, this );
  connect( mainwindow, SIGNAL(settingsDialogRequested()), settingsDialog, SLOT( show() ) );

  QSignalMapper *mapper = new QSignalMapper( this );
  mapper->setMapping( qApp, 1 );
  connect( qApp, SIGNAL(aboutToQuit()), mapper, SLOT(map()) );
  connect( mapper, SIGNAL(mapped(int)), settingsDialog, SLOT(saveConfig(int)) );

#ifdef Q_WS_HILDON
  // create menu bar only on maemo
  createMenu();
#endif
  createTrayIcon();
}

void Qtwitter::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Escape) {
    if(isVisible())
      hide();
    event->accept();
  }
  else if (event->key() == Qt::Key_F6) {
    if (isFullScreen())
      showNormal();
    else
      showFullScreen();

    event->accept();
  }
  else
    QWidget::keyPressEvent( event);
}

void Qtwitter::closeEvent( QCloseEvent *e )
{
  if ( trayIcon->isVisible()) {
    hide();
    e->ignore();
    return;
  }
  QWidget::closeEvent( e );
}


void Qtwitter::createMenu()
{
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

  QAction* aboutAction = new QAction( tr( "About qTwitter..." ), fileMenu );
  QAction* quitAction = new QAction( tr( "Quit" ), fileMenu );
  quitAction->setShortcutContext( Qt::ApplicationShortcut );
  connect( quitAction, SIGNAL(triggered()), qApp, SLOT(quit()) );

  connect( aboutAction, SIGNAL(triggered()), mainwindow, SLOT(about()) );

  fileMenu->addAction( aboutAction );
  fileMenu->addAction( quitAction );
}

void Qtwitter::setCurrentModel( TwitterAPI::SocialNetwork network, const QString &login )
{
  mainwindow->setListViewModel( core->getModel( network, login ) );
}

//  this is to avoid relying on translation files
//  caused by a bug in tr() method
void Qtwitter::setPublicTimelineModel( TwitterAPI::SocialNetwork network )
{
  mainwindow->setListViewModel( core->getPublicTimelineModel( network ) );
}

void Qtwitter::createTrayIcon()
{
  trayIcon = new QSystemTrayIcon( this );
  trayIcon->setIcon( QIcon( ":/icons/twitter_48.png" ) );

  connect( trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)) );
  connect( trayIcon, SIGNAL(messageClicked()), this, SLOT(show()) );
#ifndef Q_WS_MAC
  QMenu *trayMenu = new QMenu( this );
  trayMenu = new QMenu( this );
  QAction *quitaction = new QAction( tr( "Quit" ), trayMenu);
  QAction *settingsaction = new QAction( tr( "Settings" ), trayMenu);
  settingsaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
  quitaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ) );

  connect( quitaction, SIGNAL(triggered()), qApp, SLOT(quit()) );
  connect( settingsaction, SIGNAL(triggered()), mainwindow, SIGNAL(settingsDialogRequested()) );
  connect( settingsaction, SIGNAL(triggered()), mainwindow, SLOT(show()) );

  trayMenu->addAction(settingsaction);
  trayMenu->addSeparator();
  trayMenu->addAction(quitaction);
  trayIcon->setContextMenu( trayMenu );

  trayIcon->setToolTip( "qTwitter" );
#endif
  trayIcon->show();
}

void Qtwitter::iconActivated( QSystemTrayIcon::ActivationReason reason )
{
  switch ( reason ) {
    case QSystemTrayIcon::Trigger:
#ifdef Q_WS_WIN
    if ( !isVisible() ) {
#else
    if ( !isVisible() || !QApplication::activeWindow() ) {
#endif
      show();
        raise();
        activateWindow();
      } else {
        hide();
      }
      break;
    default:
      break;
  }
}


void Qtwitter::popupMessage( QString message )
{
  if( settings.value( "General/notifications" ).toBool() ) {
    //: The full sentence is e.g.: "New tweets for <user A>, <user B> and the public timeline"
    message.replace( "public timeline", tr( "the public timeline" ) );
    //: New tweets received (pops up in tray)
    trayIcon->showMessage( tr( "New tweets" ), message, QSystemTrayIcon::Information );
  }
}


