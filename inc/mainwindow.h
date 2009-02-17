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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settings.h"
#include "loopedsignal.h"
#include "ui_mainwindow.h"
#include "core.h"

#include <QSystemTrayIcon>

#define STATUS_MAX_LEN 140

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();
  void retranslateUi();
  
public slots:
  void popupError( const QString &message );
  void displayItem( Entry *entry );
  void deleteItem( int id );
  void setImageForUrl( const QString&, QImage );
  void setModelToBeCleared();

private slots:
  void iconActivated( QSystemTrayIcon::ActivationReason reason );
  void changeLabel();
  void sendStatus();
  void resetStatus();

signals:
  void updateTweets();
  void settingsDialogRequested();
  void post( const QByteArray& );
  void openBrowser( QString address = QString() );
  void addReplyString( const QString& );
  void destroy( int );

protected:
  void closeEvent( QCloseEvent *e );

private:
  void resizeEvent( QResizeEvent* );
  Core* core;
  QMenu *menu;
  QSystemTrayIcon *trayIcon;
  QStandardItemModel model;
  Settings *settingsDialog;
  bool modelToBeCleared;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
