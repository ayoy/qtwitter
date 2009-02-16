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
#include "statusfilter.h"
#include "tweet.h"

#include <QMenu>
#include <QScrollBar>
#include <QMessageBox>
#include <QIcon>

MainWindow::MainWindow() : QWidget(), model( 0, 0, this )
{
  ui.setupUi( this );
  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + " " + tr( "characters left" ) );
  StatusFilter *filter = new StatusFilter( this );
  ui.statusEdit->installEventFilter( filter );
  ui.statusListView->setModel( &model );

  connect( ui.updateButton, SIGNAL( clicked() ), this, SIGNAL( updateTweets() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), this, SIGNAL(settingsDialogRequested()) );
  connect( ui.homeButton, SIGNAL(clicked()), this, SIGNAL(openBrowser()) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( filter, SIGNAL( escPressed() ), ui.statusEdit, SLOT( cancelEditing() ) );
  connect( ui.statusListView, SIGNAL( contextMenuRequested() ), this, SLOT( popupMenu() ) );
  connect( this, SIGNAL(addReplyString(QString)), ui.statusEdit, SLOT(addReplyString(QString)) );

  trayIcon = new QSystemTrayIcon( this );
  trayIcon->setIcon( QIcon( ":/icons/twitter_48.png" ) );

  QObject::connect( trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)) );
#ifndef Q_WS_MAC
  QMenu *trayMenu = new QMenu( this );
  QAction *showaction = new QAction( tr( "Show" ), trayMenu);
  QAction *settingsaction = new QAction( tr( "Settings" ), trayMenu);
  QAction *quitaction = new QAction( tr( "Quit" ), trayMenu);

  QObject::connect( showaction, SIGNAL(triggered()), this, SLOT(show()) );
  QObject::connect( quitaction, SIGNAL(triggered()), qApp, SLOT(quit()) );
  QObject::connect( settingsaction, SIGNAL(triggered()), this, SIGNAL(settingsDialogRequested()) );

  trayMenu->addAction(showaction);
  trayMenu->addAction(settingsaction);
  trayMenu->addAction(quitaction);
  trayIcon->setContextMenu( trayMenu );

  trayIcon->setToolTip( "qTwitter" );
#endif
  trayIcon->show();
}

void MainWindow::popupMenu()
{
//  menu->exec( QCursor::pos() );
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent( QCloseEvent *e )
{
  if ( trayIcon->isVisible()) {
    hide();
    e->ignore();
    return;
  }
  QWidget::closeEvent( e );
}

void MainWindow::iconActivated( QSystemTrayIcon::ActivationReason reason )
{
  switch ( reason ) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
      if ( !isVisible() )
        showNormal();
      else
        hide();
      break;
    default:
      break;
  }
}


void MainWindow::changeLabel()
{
  ui.countdownLabel->setText( ui.statusEdit->isStatusClean() ? QString::number( STATUS_MAX_LEN ) : QString::number( STATUS_MAX_LEN - ui.statusEdit->text().length() ) );
  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + " " + tr( "characters left" ) );
}

void MainWindow::sendStatus()
{
  emit post( ui.statusEdit->text().toUtf8() );
}

void MainWindow::resetStatus()
{
  if ( ui.statusEdit->isStatusClean() ) {
    changeLabel();
  }
}

void MainWindow::resizeEvent( QResizeEvent *event )
{
  if ( model.rowCount() == 0 )
    return;

  QSize itemSize;
  int scrollBarMargin = ui.statusListView->verticalScrollBar()->size().width();
  for ( int i = 0; i < model.rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( ui.statusListView->indexWidget( model.indexFromItem( model.item(i) ) ) );
    aTweet->resize( event->size().width() - scrollBarMargin, aTweet->size().height() );
    itemSize = model.item(i)->sizeHint();
    itemSize.rwidth() += event->size().width() - event->oldSize().width();
    itemSize.rheight() = aTweet->size().height();
    model.item(i)->setSizeHint( itemSize );
  }
}

void MainWindow::displayItem( const Entry &entry )
{
  if ( modelToBeCleared ) {
    model.clear();
    modelToBeCleared = false;
  }
  qDebug() << entry.getId() << entry.name();
  int scrollBarMargin = ui.statusListView->verticalScrollBar()->size().width();
  QStandardItem *newItem = new QStandardItem();
  Tweet *newTweet = new Tweet( entry, QImage(), this );
  newTweet->resize( ui.statusListView->width() - scrollBarMargin, newTweet->size().height() );
  newItem->setSizeHint( newTweet->size() );
  model.insertRow( entry.getId(), newItem );
  ui.statusListView->setIndexWidget( model.indexFromItem( newItem ), newTweet );
}

void MainWindow::setModelToBeCleared()
{
  modelToBeCleared = true;
}

void MainWindow::setImageForUrl( const QString& url, QImage image )
{
  for ( int i = 0; i < model.rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( ui.statusListView->indexWidget( model.indexFromItem( model.item(i) ) ) );
    if ( !aTweet->getUrlForIcon().compare( url ) ) {
      aTweet->setIcon( image );
    }
  }
}

void MainWindow::popupError( const QString &message )
{
  QMessageBox::information( this, tr("Error"), message );
}

void MainWindow::retranslateUi()
{
  ui.homeButton->setToolTip( tr("Go to twitter.com") );
  ui.settingsButton->setToolTip( tr("Settings") );
  ui.updateButton->setToolTip( tr("Update tweets") );
  if ( ui.statusEdit->isStatusClean() ) {
    ui.statusEdit->initialize();
  }
  ui.statusEdit->setText( tr("What are you doing?") );
  for ( int i = 0; i < model.rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( ui.statusListView->indexWidget( model.indexFromItem( model.item(i) ) ) );
    aTweet->retranslateUi();
  }
}
