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
  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + tr( " characters left" ) );
  StatusFilter *filter = new StatusFilter( this );
  ui.statusEdit->installEventFilter( filter );
  ui.statusListView->setModel( &model );

//  menu = new QMenu( this );
//  QAction *openaction = new QAction("Open", this);
//  QAction *closeaction = new QAction("Close", this);
//  QAction *deleteaction = new QAction("Delete", this);
//  QAction *aboutaction = new QAction("About", this);

//  menu->addAction(openaction);
//  menu->addAction(closeaction);
//  menu->addAction(deleteaction);
//  menu->addAction(aboutaction);

  qDebug() << qRegisterMetaType<ListOfEntries>( "ListOfEntries" );
  qDebug() << qRegisterMetaType<MapStringImage>( "MapStringImage" );

  connect( ui.updateButton, SIGNAL( clicked() ), this, SLOT( updateTweets() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), this, SIGNAL(settingsDialogRequested()) );
  connect( ui.homeButton, SIGNAL(clicked()), this, SIGNAL(openBrowser()) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( filter, SIGNAL( escPressed() ), ui.statusEdit, SLOT( cancelEditing() ) );
  connect( ui.statusListView, SIGNAL( contextMenuRequested() ), this, SLOT( popupMenu() ) );

  trayIcon = new QSystemTrayIcon( this );
  trayIcon->setIcon( QIcon( ":/icons/twitter_48.png" ) );

  QObject::connect( trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)) );
#ifndef Q_WS_MAC
  QMenu *trayMenu = new QMenu( this );
  QAction *showaction = new QAction("Show", trayMenu);
  QAction *settingsaction = new QAction("Settings", trayMenu);
  QAction *quitaction = new QAction("Quit", trayMenu);

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
  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + tr( " characters left" ) );
}

void MainWindow::updateTweets()
{
  ui.updateButton->setEnabled( false );
  ui.statusEdit->setEnabled( false );
  emit get();
}

void MainWindow::sendStatus()
{
  ui.updateButton->setEnabled( false );
  ui.statusEdit->setEnabled( false );
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

void MainWindow::display( const ListOfEntries &entries, const MapStringImage &imagesHash )
{
  model.clear();
  int scrollBarMargin = ui.statusListView->verticalScrollBar()->size().width();
  for ( int i = 0; i < entries.size(); i++ ) {
    QStandardItem *newItem = new QStandardItem();
    Tweet *newTweet = new Tweet( entries[i].name(), entries[i].text(), imagesHash[ entries[i].image() ], this );
    newTweet->resize( ui.statusListView->width() - scrollBarMargin, newTweet->size().height() );
    newItem->setSizeHint( newTweet->size() );
    model.appendRow( newItem );
    ui.statusListView->setIndexWidget( model.indexFromItem( newItem ), newTweet );
  }
  unlock();
}

void MainWindow::unlock()
{
  ui.updateButton->setEnabled( true );
  if ( !ui.statusEdit->isEnabled() ) {
    ui.statusEdit->setEnabled( true );
    ui.statusEdit->initialize();
  }
}

void MainWindow::popupError( const QString &message )
{
  QMessageBox::information( this, tr("Error"), message );
  unlock();
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
}
