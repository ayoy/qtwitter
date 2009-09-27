/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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


#include <QApplication>
#include "qtwitter.h"

#ifdef QT_DBUS
#   include <QDBusConnection>
#endif

int main( int argc, char **argv )
{
  QApplication app( argc, argv );

#ifdef QT_DBUS
  QDBusConnection connection = QDBusConnection::sessionBus();
  bool res = connection.registerService( "net.ayoy.qTwitter" );
  if ( !res ) {
    return 1;
  }
#endif

  qApp->setWindowIcon( QIcon( ":/icons/twitter_48.png" ) );
  Qtwitter qtwitter;
  QApplication::setQuitOnLastWindowClosed( false );

  qtwitter.show();
  return app.exec();
}
