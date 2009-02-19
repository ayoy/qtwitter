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


#include "mainwindow.h"
#include "tweetmodel.h"
#include "settings.h"
#include "core.h"
#include "loopedsignal.h"

//Q_IMPORT_PLUGIN(qjpeg)
//Q_IMPORT_PLUGIN(qgif)


int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  qApp->setWindowIcon( QIcon( ":/icons/twitter_48.png" ) );

  MainWindow qtwitter;
  TweetModel *model = new TweetModel( qtwitter.getScrollBarWidth(), qtwitter.getListView(), &qtwitter );
  qtwitter.setListViewModel( model );

  LoopedSignal *loopedsignal = new LoopedSignal( &qtwitter );
  Core *core = new Core( &qtwitter );
  Settings *settings = new Settings( model, &qtwitter, loopedsignal, core, &qtwitter );

  QObject::connect( loopedsignal, SIGNAL(ping()), core, SLOT(get()) );
  QObject::connect( &qtwitter, SIGNAL(updateTweets()), core, SLOT(get()) );
  QObject::connect( &qtwitter, SIGNAL(openBrowser(QString)), core, SLOT(openBrowser(QString)) );
  QObject::connect( &qtwitter, SIGNAL(post(QByteArray)), core, SLOT(post(QByteArray)) );
  QObject::connect( &qtwitter, SIGNAL(settingsDialogRequested()), settings, SLOT( show() ) );
  QObject::connect( &qtwitter, SIGNAL(destroy(int)), core, SLOT(destroyTweet(int)) );
  QObject::connect( &qtwitter, SIGNAL(resizeView(int,int)), model, SLOT(resizeData(int,int)));
  QObject::connect( core, SIGNAL(authDataSet(QAuthenticator)), settings, SLOT(setAuthDataInDialog(QAuthenticator)) ) ;
  QObject::connect( core, SIGNAL(switchToPublic()), settings, SLOT(switchToPublic()) );
  QObject::connect( core, SIGNAL(errorMessage(QString)), &qtwitter, SLOT(popupError(QString)) );
  QObject::connect( core, SIGNAL(addOneEntry(Entry*)), model, SLOT(insertTweet(Entry*)) );
  QObject::connect( core, SIGNAL(deleteEntry(int)), model, SLOT(deleteTweet(int)) );
  QObject::connect( core, SIGNAL(setImageForUrl(QString,QImage)), model, SLOT(setImageForUrl(QString,QImage)) );
  QObject::connect( core, SIGNAL(requestListRefresh()), model, SLOT(setModelToBeCleared()) );
//  QObject::connect( settings, SIGNAL(languageChanged()), &qtwitter, SLOT(retranslateUi()) );
//  QObject::connect( settings, SIGNAL(languageChanged()), model, SLOT(retranslateUi()) );
  QObject::connect( qApp, SIGNAL(aboutToQuit()), settings, SLOT(saveConfig()) );
  QObject::connect( &qtwitter, SIGNAL(ready()), loopedsignal, SLOT(start()) );

  qtwitter.show();

  return app.exec();
}
