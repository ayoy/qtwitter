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

#include "twitterapi.h"
#include "mainwindow.h"
#include "tweetmodel.h"
#include "twitpicview.h"
#include "settings.h"
#include "core.h"
#include <QSystemTrayIcon>
#include <QSignalMapper>

int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  qApp->setWindowIcon( QIcon( ":/icons/twitter_48.png" ) );

  MainWindow qtwitter;
  TweetModel *model = new TweetModel( qtwitter.getScrollBarWidth(), qtwitter.getListView(), &qtwitter );
  qtwitter.setListViewModel( model );

  TwitPicView *twitpic = new TwitPicView( &qtwitter );
  Core *core = new Core( &qtwitter );
  Settings *settings = new Settings( model, &qtwitter, core, &qtwitter );

  QObject::connect( &qtwitter, SIGNAL(updateTweets()), core, SLOT(forceGet()) );
  QObject::connect( &qtwitter, SIGNAL(openBrowser(QUrl)), core, SLOT(openBrowser(QUrl)) );
  QObject::connect( &qtwitter, SIGNAL(openTwitPicDialog()), twitpic, SLOT(show()) );
  QObject::connect( model, SIGNAL(openBrowser(QUrl)), core, SLOT(openBrowser(QUrl)) );
  QObject::connect( model, SIGNAL(reply(QString,int)), &qtwitter, SIGNAL(addReplyString(QString,int)) );
  QObject::connect( model, SIGNAL(about()), &qtwitter, SLOT(about()) );
  QObject::connect( model, SIGNAL(destroy(int)), core, SLOT(destroyTweet(int)) );
  QObject::connect( model, SIGNAL(retweet(QString)), &qtwitter, SIGNAL(addRetweetString(QString)) );
  QObject::connect( &qtwitter, SIGNAL(post(QByteArray,int)), core, SLOT(post(QByteArray,int)) );
  QObject::connect( twitpic, SIGNAL(uploadPhoto(QString,QString)), core, SLOT(uploadPhoto(QString,QString)) );
  QObject::connect( twitpic, SIGNAL(abortUpload()), core, SLOT(abortUploadPhoto()) );
  QObject::connect( core, SIGNAL(twitPicResponseReceived()), twitpic, SLOT(resetForm()) );
  QObject::connect( core, SIGNAL(twitPicDataSendProgress(int,int)), twitpic, SLOT(showUploadProgress(int,int)) );
  QObject::connect( &qtwitter, SIGNAL(settingsDialogRequested()), settings, SLOT( show() ) );
  QObject::connect( &qtwitter, SIGNAL(resizeView(int,int)), model, SLOT(resizeData(int,int)));
  QObject::connect( core, SIGNAL(authDataSet(QAuthenticator)), settings, SLOT(setAuthDataInDialog(QAuthenticator)) ) ;
  QObject::connect( core, SIGNAL(errorMessage(QString)), &qtwitter, SLOT(popupError(QString)) );
  QObject::connect( core, SIGNAL(addEntry(Entry*)), model, SLOT(insertTweet(Entry*)) );
  QObject::connect( core, SIGNAL(deleteEntry(int)), model, SLOT(deleteTweet(int)) );
  QObject::connect( core, SIGNAL(setImageForUrl(QString,QImage)), model, SLOT(setImageForUrl(QString,QImage)) );
  QObject::connect( core, SIGNAL(requestListRefresh(bool,bool)), model, SLOT(setModelToBeCleared(bool,bool)) );
  QObject::connect( core, SIGNAL(timelineUpdated()), model, SLOT(sendNewsInfo()) );
  QObject::connect( core, SIGNAL(resetUi()), &qtwitter, SLOT(resetStatusEdit()) );
  QObject::connect( core, SIGNAL(requestStarted()), &qtwitter, SLOT(showProgressIcon()) );

  QObject::connect( core, SIGNAL(publicTimelineSyncChanged(bool)), settings, SLOT(slotPublicTimelineSyncChanged(bool)) );
  QObject::connect( core, SIGNAL(directMessagesSyncChanged(bool)), model, SLOT(slotDirectMessagesChanged(bool)) );

  if ( QSystemTrayIcon::supportsMessages() ) {
    QObject::connect( model, SIGNAL(newTweets(int,QStringList,int,QStringList)), &qtwitter, SLOT(popupMessage(int,QStringList,int,QStringList)) );
  }
  QSignalMapper mapper;
  mapper.setMapping( qApp, 1 );
  QObject::connect( qApp, SIGNAL(aboutToQuit()), &mapper, SLOT(map()) );
  QObject::connect( &mapper, SIGNAL(mapped(int)), settings, SLOT(saveConfig(int)) );

  QApplication::setQuitOnLastWindowClosed( false );

  qtwitter.show();

  return app.exec();
}
