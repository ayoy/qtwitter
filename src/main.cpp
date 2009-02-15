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
  LoopedSignal *loopedsignal = new LoopedSignal( &qtwitter );
  Core *core = new Core( &qtwitter );
  Settings *settings = new Settings( &qtwitter, loopedsignal, core, &qtwitter );

  QObject::connect( loopedsignal, SIGNAL(ping()), core, SLOT(get()) );
  QObject::connect( &qtwitter, SIGNAL(updateTweets()), core, SLOT(get()) );
  QObject::connect( &qtwitter, SIGNAL(openBrowser(QString)), core, SLOT(openBrowser(QString)) );
  QObject::connect( &qtwitter, SIGNAL(post(QByteArray)), core, SLOT(post(QByteArray)) );
  QObject::connect( &qtwitter, SIGNAL(settingsDialogRequested()), settings, SLOT( show() ) );
  QObject::connect( core, SIGNAL(authDataSet(QAuthenticator)), settings, SLOT(setAuthDataInDialog(QAuthenticator)) ) ;
  QObject::connect( core, SIGNAL(switchToPublic()), settings, SLOT(switchToPublic()) );
  QObject::connect( core, SIGNAL(errorMessage(QString)), &qtwitter, SLOT(popupError(QString)) );
  QObject::connect( core, SIGNAL(addOneEntry(Entry)), &qtwitter, SLOT(displayItem(Entry)) );
  QObject::connect( core, SIGNAL(setImageForUrl(QString,QImage)), &qtwitter, SLOT(setImageForUrl(QString,QImage)) );
  QObject::connect( core, SIGNAL(requestListRefresh()), &qtwitter, SLOT(setModelToBeCleared()) );

  qtwitter.show();

  if ( !loopedsignal->isRunning() )
    loopedsignal->start();

  return app.exec();
}
