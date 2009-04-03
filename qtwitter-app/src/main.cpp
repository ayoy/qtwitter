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

#include "qtwitter.h"

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

  Qtwitter qtwitter;

  QApplication::setQuitOnLastWindowClosed( false );

  qtwitter.show();

  return app.exec();
}
