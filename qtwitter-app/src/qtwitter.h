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


#ifndef QTWITTER_H
#define QTWITTER_H

#include <QMainWindow>
#include "settings.h"

class Core;
class MainWindow;
class QSystemTrayIcon;
class TwitPicView;

class Qtwitter : public QMainWindow
{
  Q_OBJECT
public:
  Qtwitter( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

public slots:
  void popupMessage( QString message );
  void setCurrentModel( const QString &login );
  void setPublicTimelineModel();

protected:
  void closeEvent( QCloseEvent *e );
  void keyPressEvent ( QKeyEvent * event );

private:
  void createMenu();
  void createTrayIcon();

  Core *core;
  MainWindow* mainwindow;
  QSystemTrayIcon *trayIcon;
  TwitPicView *twitpic;
  Settings *settingsDialog;
};

#endif // QTWITTER_H
