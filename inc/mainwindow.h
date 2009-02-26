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

#include "ui_mainwindow.h"
#include "tweetmodel.h"

#include <QSystemTrayIcon>
#include <QModelIndex>

class TweetModel;

class MainWindow : public QWidget
{
  Q_OBJECT

  static const QString APP_VERSION;

public:
  MainWindow( QWidget *parent = 0 );
  ~MainWindow();
  StatusList* getListView();
  int getScrollBarWidth();
  void setListViewModel( TweetModel *model );
  
public slots:
  void popupError( const QString &message );
  void retranslateUi();
  void changeListBackgroundColor( const QColor &newColor );
  void popupMessage( int statusesCount, QStringList namesForStatuses, int messagesCount, QStringList namesForMessages );
  void about();
  void resetStatusEdit();

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
  void resizeView( int width, int oldWidth );

protected:
  void closeEvent( QCloseEvent *e );

private:
  bool resetUiWhenFinished;
  void resizeEvent( QResizeEvent* );
  QMenu *trayMenu;
  QSystemTrayIcon *trayIcon;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
