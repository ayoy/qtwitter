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

#include <QSystemTrayIcon>
#include <QSignalMapper>
#include <QSettings>
#include "qtwitter.h"
#include "core.h"
#include "twitpicview.h"
#include "settings.h"
#include "twitteraccountsmodel.h"

Qtwitter::Qtwitter( QWidget *parent ) : MainWindow( parent )
{
  core = new Core( this );
//  setListViewModel( core->getModel() );
  twitpic = new TwitPicView( this );
  settingsDialog = new Settings( this, core, this );

  connect( core, SIGNAL(modelChanged(TweetModel*)), this, SLOT(setListViewModel(TweetModel*)) );

  connect( this, SIGNAL(updateTweets()), core, SLOT(forceGet()) );
  connect( this, SIGNAL(openBrowser(QUrl)), core, SLOT(openBrowser(QUrl)) );
  connect( this, SIGNAL(post(QString,int)), core, SLOT(post(QString,int)) );
  connect( this, SIGNAL(resizeView(int,int)), core, SIGNAL(resizeData(int,int)));
  connect( core, SIGNAL(about()), this, SLOT(about()) );
  connect( core, SIGNAL(addReplyString(QString,int)), this, SIGNAL(addReplyString(QString,int)) );
  connect( core, SIGNAL(addRetweetString(QString)), this, SIGNAL(addRetweetString(QString)) );
  connect( core, SIGNAL(errorMessage(QString)), this, SLOT(popupError(QString)) );
  connect( core, SIGNAL(resetUi()), this, SLOT(resetStatusEdit()) );
  connect( core, SIGNAL(requestStarted()), this, SLOT(showProgressIcon()) );
  if ( QSystemTrayIcon::supportsMessages() ) {
    connect( core, SIGNAL(newTweets(int,QStringList,int,QStringList)), this, SLOT(popupMessage(int,QStringList,int,QStringList)) );
  }


  connect( this, SIGNAL(settingsDialogRequested()), settingsDialog, SLOT( show() ) );
  connect( settingsDialog, SIGNAL(accountsChanged(QList<TwitterAccount>,bool)), this, SLOT(setupTwitterAccounts(QList<TwitterAccount>,bool)) );

  connect( this, SIGNAL(openTwitPicDialog()), twitpic, SLOT(show()) );

  connect( twitpic, SIGNAL(uploadPhoto(QString,QString)), core, SLOT(uploadPhoto(QString,QString)) );
  connect( twitpic, SIGNAL(abortUpload()), core, SLOT(abortUploadPhoto()) );
  connect( core, SIGNAL(twitPicResponseReceived()), twitpic, SLOT(resetForm()) );
  connect( core, SIGNAL(twitPicDataSendProgress(int,int)), twitpic, SLOT(showUploadProgress(int,int)) );

  connect( core, SIGNAL(authDataSet(QAuthenticator)), settingsDialog, SLOT(setAuthDataInDialog(QAuthenticator)) ) ;
  connect( core, SIGNAL(publicTimelineSyncChanged(bool)), settingsDialog, SLOT(slotPublicTimelineSyncChanged(bool)) );

  QSignalMapper *mapper = new QSignalMapper( this );
  mapper->setMapping( qApp, 1 );
  connect( qApp, SIGNAL(aboutToQuit()), mapper, SLOT(map()) );
  connect( mapper, SIGNAL(mapped(int)), settingsDialog, SLOT(saveConfig(int)) );
}
