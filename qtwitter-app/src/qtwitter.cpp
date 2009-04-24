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


#include <QSystemTrayIcon>
#include <QSignalMapper>
#include <QSettings>
#include "qtwitter.h"
#include "core.h"
#include "twitpicview.h"
#include "tweet.h"
#include "settings.h"
#include "twitteraccountsmodel.h"

Qtwitter::Qtwitter( QWidget *parent ) : MainWindow( parent )
{
  connect( this, SIGNAL(switchModel(QString)), SLOT(setCurrentModel(QString)) );
  connect( this, SIGNAL(switchToPublicTimelineModel()), SLOT(setPublicTimelineModel()) );

  core = new Core( this );
  twitpic = new TwitPicView( this );
  settingsDialog = new Settings( this, core, twitpic, this );

  connect( this, SIGNAL(updateTweets()), core, SLOT(forceGet()) );
  connect( this, SIGNAL(openBrowser(QUrl)), core, SLOT(openBrowser(QUrl)) );
  connect( this, SIGNAL(post(QString,QString,int)), core, SLOT(post(QString,QString,int)) );
  connect( this, SIGNAL(resizeView(int,int)), core, SIGNAL(resizeData(int,int)));
  connect( this, SIGNAL(shortenUrl(QString)), core, SLOT(shortenUrl(QString)));
  connect( core, SIGNAL(twitterAccountsChanged(QList<TwitterAccount>,bool)), this, SLOT(setupTwitterAccounts(QList<TwitterAccount>,bool)) );
  connect( core, SIGNAL(urlShortened(QString)), this, SLOT(replaceUrl(QString)));
  connect( core, SIGNAL(about()), this, SLOT(about()) );
  connect( core, SIGNAL(addReplyString(QString,int)), this, SIGNAL(addReplyString(QString,int)) );
  connect( core, SIGNAL(addRetweetString(QString)), this, SIGNAL(addRetweetString(QString)) );
  connect( core, SIGNAL(errorMessage(QString)), this, SLOT(popupError(QString)) );
  connect( core, SIGNAL(resetUi()), this, SLOT(resetStatusEdit()) );
  connect( core, SIGNAL(requestStarted()), this, SLOT(showProgressIcon()) );
  if ( QSystemTrayIcon::supportsMessages() )
    connect( core, SIGNAL(sendNewsReport(QString)), this, SLOT(popupMessage(QString)) );

  connect( this, SIGNAL(settingsDialogRequested()), settingsDialog, SLOT( show() ) );

  connect( twitpic, SIGNAL(uploadPhoto(QString,QString,QString)), core, SLOT(uploadPhoto(QString,QString,QString)) );
  connect( twitpic, SIGNAL(abortUpload()), core, SLOT(abortUploadPhoto()) );
  connect( this, SIGNAL(openTwitPicDialog()), twitpic, SLOT(show()) );
  connect( core, SIGNAL(twitPicResponseReceived()), twitpic, SLOT(resetForm()) );
  connect( core, SIGNAL(twitPicDataSendProgress(int,int)), twitpic, SLOT(showUploadProgress(int,int)) );

  QSignalMapper *mapper = new QSignalMapper( this );
  mapper->setMapping( qApp, 1 );
  connect( qApp, SIGNAL(aboutToQuit()), mapper, SLOT(map()) );
  connect( mapper, SIGNAL(mapped(int)), settingsDialog, SLOT(saveConfig(int)) );
}

void Qtwitter::setCurrentModel( const QString &login )
{
  setListViewModel( core->getModel( login ) );
}

//  this is to avoid relying on translation files
//  caused by a bug in tr() method
void Qtwitter::setPublicTimelineModel()
{
  setListViewModel( core->getPublicTimelineModel() );
}
