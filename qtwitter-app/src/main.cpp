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
#include "twitterapi.h"
#include "mainwindow.h"
#include "tweetmodel.h"
#include "twitpicview.h"
#include "settings.h"
#include "core.h"


int main( int argc, char **argv )
{
  QCoreApplication::setOrganizationName( "ayoy" );
  QCoreApplication::setOrganizationDomain( "ayoy.net" );
  QCoreApplication::setApplicationName( "qTwitter" );
#if defined Q_WS_WIN
  QSettings::setDefaultFormat( QSettings::IniFormat );
#endif
  QApplication app( argc, argv );
  qApp->setWindowIcon( QIcon( ":/icons/twitter_48.png" ) );

  MainWindow qtwitter;
  Core *core = new Core( &qtwitter );
  qtwitter.setListViewModel( core->getModel() );
  TwitPicView *twitpic = new TwitPicView( &qtwitter );

  Settings *settingsDialog = new Settings( &qtwitter, core, &qtwitter );

  QObject::connect( core, SIGNAL(modelChanged(QStandardItemModel*)), &qtwitter, SLOT(setListViewModel(QStandardItemModel*)) );

  QObject::connect( &qtwitter, SIGNAL(updateTweets()), core, SLOT(forceGet()) );
  QObject::connect( &qtwitter, SIGNAL(openBrowser(QUrl)), core, SLOT(openBrowser(QUrl)) );
  QObject::connect( &qtwitter, SIGNAL(openTwitPicDialog()), twitpic, SLOT(show()) );
  QObject::connect( core, SIGNAL(about()), &qtwitter, SLOT(about()) );
  QObject::connect( core, SIGNAL(addReplyString(QString,int)), &qtwitter, SIGNAL(addReplyString(QString,int)) );
  QObject::connect( core, SIGNAL(addRetweetString(QString)), &qtwitter, SIGNAL(addRetweetString(QString)) );

  QObject::connect( &qtwitter, SIGNAL(post(QByteArray,int)), core, SLOT(post(QByteArray,int)) );
  QObject::connect( twitpic, SIGNAL(uploadPhoto(QString,QString)), core, SLOT(uploadPhoto(QString,QString)) );
  QObject::connect( twitpic, SIGNAL(abortUpload()), core, SLOT(abortUploadPhoto()) );
  QObject::connect( core, SIGNAL(twitPicResponseReceived()), twitpic, SLOT(resetForm()) );
  QObject::connect( core, SIGNAL(twitPicDataSendProgress(int,int)), twitpic, SLOT(showUploadProgress(int,int)) );
  QObject::connect( &qtwitter, SIGNAL(settingsDialogRequested()), settingsDialog, SLOT( show() ) );
  QObject::connect( &qtwitter, SIGNAL(resizeView(int,int)), core, SIGNAL(resizeData(int,int)));
  QObject::connect( core, SIGNAL(authDataSet(QAuthenticator)), settingsDialog, SLOT(setAuthDataInDialog(QAuthenticator)) ) ;
  QObject::connect( core, SIGNAL(errorMessage(QString)), &qtwitter, SLOT(popupError(QString)) );
  QObject::connect( core, SIGNAL(resetUi()), &qtwitter, SLOT(resetStatusEdit()) );
  QObject::connect( core, SIGNAL(requestStarted()), &qtwitter, SLOT(showProgressIcon()) );

  QObject::connect( core, SIGNAL(publicTimelineSyncChanged(bool)), settingsDialog, SLOT(slotPublicTimelineSyncChanged(bool)) );


  if ( QSystemTrayIcon::supportsMessages() ) {
    QObject::connect( core, SIGNAL(newTweets(int,QStringList,int,QStringList)), &qtwitter, SLOT(popupMessage(int,QStringList,int,QStringList)) );
  }
  QSignalMapper mapper;
  mapper.setMapping( qApp, 1 );
  QObject::connect( qApp, SIGNAL(aboutToQuit()), &mapper, SLOT(map()) );
  QObject::connect( &mapper, SIGNAL(mapped(int)), settingsDialog, SLOT(saveConfig(int)) );

  QApplication::setQuitOnLastWindowClosed( false );

  qtwitter.show();

  return app.exec();
}
