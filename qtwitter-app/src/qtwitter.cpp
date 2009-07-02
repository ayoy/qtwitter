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
#include "statuswidget.h"
#include "settings.h"
#include "account.h"

Qtwitter::Qtwitter( QWidget *parent ) :
    MainWindow( parent ),
    twitpic(0)
{
  connect( this, SIGNAL(switchModel(TwitterAPI::SocialNetwork,QString)), SLOT(setCurrentModel(TwitterAPI::SocialNetwork,QString)) );
  connect( this, SIGNAL(twitPicRequested()), SLOT(openTwitPic()) );

  core = new Core( this );

  connect( this, SIGNAL(updateStatuses()), core, SLOT(forceGet()) );
  connect( this, SIGNAL(openBrowser(QUrl)), core, SLOT(openBrowser(QUrl)) );
  connect( this, SIGNAL(post(TwitterAPI::SocialNetwork,QString,QString,quint64)), core, SLOT(post(TwitterAPI::SocialNetwork,QString,QString,quint64)) );
  connect( this, SIGNAL(resizeView(int,int)), core, SIGNAL(resizeData(int,int)));
  connect( this, SIGNAL(shortenUrl(QString)), core, SLOT(shortenUrl(QString)));
  connect( this, SIGNAL(iconStopped()), core, SLOT(resetRequestsCount()) );
  connect( this, SIGNAL(statusMarkeverythingasreadAction()), core, SLOT(markEverythingAsRead()) );
  connect( core, SIGNAL(pauseIcon()), this, SLOT(pauseIcon()) );
  connect( core, SIGNAL(accountsUpdated(QList<Account>)), this, SLOT(setupAccounts(QList<Account>)) );
  connect( core, SIGNAL(urlShortened(QString)), this, SLOT(replaceUrl(QString)));
  connect( core, SIGNAL(about()), this, SLOT(about()) );
  connect( core, SIGNAL(addReplyString(QString,quint64)), this, SIGNAL(addReplyString(QString,quint64)) );
  connect( core, SIGNAL(addRetweetString(QString)), this, SIGNAL(addRetweetString(QString)) );
  connect( core, SIGNAL(errorMessage(QString)), this, SLOT(popupError(QString)) );
  connect( core, SIGNAL(resetUi()), this, SLOT(resetStatusEdit()) );
  connect( core, SIGNAL(requestStarted()), this, SLOT(showProgressIcon()) );
  if ( QSystemTrayIcon::supportsMessages() )
    connect( core, SIGNAL(sendNewsReport(QString)), this, SLOT(popupMessage(QString)) );


  settingsDialog = new Settings( this, core, this );
  connect( this, SIGNAL(settingsDialogRequested()), settingsDialog, SLOT( show() ) );

  QSignalMapper *mapper = new QSignalMapper( this );
  mapper->setMapping( qApp, 1 );
  connect( qApp, SIGNAL(aboutToQuit()), mapper, SLOT(map()) );
  connect( mapper, SIGNAL(mapped(int)), settingsDialog, SLOT(saveConfig(int)) );
}

void Qtwitter::setCurrentModel( TwitterAPI::SocialNetwork network, const QString &login )
{
  core->setModelData( network, login );
}

void Qtwitter::openTwitPic()
{
  if ( !twitpic ) {
    twitpic = new TwitPicView( this );
    connect( twitpic, SIGNAL(uploadPhoto(QString,QString,QString)), core, SLOT(uploadPhoto(QString,QString,QString)) );
    connect( twitpic, SIGNAL(abortUpload()), core, SLOT(abortUploadPhoto()) );
    connect( core, SIGNAL(twitPicResponseReceived()), twitpic, SLOT(resetForm()) );
    connect( core, SIGNAL(twitPicDataSendProgress(qint64,qint64)), twitpic, SLOT(showUploadProgress(qint64,qint64)) );
    connect( core, SIGNAL(accountsUpdated(QList<Account>)), twitpic, SLOT(setupAccounts(QList<Account>)) );
    twitpic->setupAccounts( core->twitpicLogins() );
  }
  twitpic->show();
}
