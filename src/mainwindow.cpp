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
#include "ui_about.h"

#include <QMenu>
#include <QScrollBar>
#include <QMessageBox>
#include <QIcon>
#include <QPalette>
#include <QShortcut>

MainWindow::MainWindow( QWidget *parent ) : QWidget( parent )
{
  ui.setupUi( this );

  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + " " + tr( "characters left" ) );
  StatusFilter *filter = new StatusFilter( this );
  ui.statusEdit->installEventFilter( filter );

  connect( ui.updateButton, SIGNAL( clicked() ), this, SIGNAL( updateTweets() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), this, SIGNAL(settingsDialogRequested()) );
  connect( ui.homeButton, SIGNAL(clicked()), this, SIGNAL(openBrowser()) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( filter, SIGNAL( escPressed() ), ui.statusEdit, SLOT( cancelEditing() ) );
  connect( this, SIGNAL(addReplyString(QString)), ui.statusEdit, SLOT(addReplyString(QString)) );
  connect( this, SIGNAL(resetStatusEdit()), ui.statusEdit, SLOT(cancelEditing()) );

  QShortcut *typeShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ), this );
  QShortcut *hideShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_H ), this );
  connect( hideShortcut, SIGNAL(activated()), this, SLOT(hide()) );
#ifdef Q_WS_MAC
  ui.settingsButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Comma ) );
  QShortcut *quitShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ), this );
  connect( quitShortcut, SIGNAL(activated()), qApp, SLOT(quit()) );
#endif
  ui.updateButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ) );
  ui.homeButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
  connect( typeShortcut, SIGNAL(activated()), ui.statusEdit, SLOT(setFocus()) );

  trayIcon = new QSystemTrayIcon( this );
  trayIcon->setIcon( QIcon( ":/icons/twitter_48.png" ) );

  QObject::connect( trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)) );
#ifndef Q_WS_MAC
  QMenu *trayMenu = new QMenu( this );
  trayMenu = new QMenu( this );
  QAction *showaction = new QAction( tr( "Show" ), trayMenu);
  QAction *quitaction = new QAction( tr( "Quit" ), trayMenu);
  QAction *settingsaction = new QAction( tr( "Settings" ), trayMenu);
  settingsaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
  quitaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ) );

  QObject::connect( showaction, SIGNAL(triggered()), this, SLOT(show()) );
  QObject::connect( quitaction, SIGNAL(triggered()), qApp, SLOT(quit()) );
  QObject::connect( settingsaction, SIGNAL(triggered()), this, SIGNAL(settingsDialogRequested()) );
  QObject::connect( settingsaction, SIGNAL(triggered()), this, SLOT(show()) );

  trayMenu->addAction(showaction);
  trayMenu->addAction(settingsaction);
  trayMenu->addAction(quitaction);
  trayIcon->setContextMenu( trayMenu );

  trayIcon->setToolTip( "qTwitter" );
#endif
  trayIcon->show();
  emit updateTweets();
}

MainWindow::~MainWindow() {}

QListView* MainWindow::getListView()
{
  return ui.statusListView;
}

int MainWindow::getScrollBarWidth()
{
  return ui.statusListView->verticalScrollBar()->size().width();
}

void MainWindow::setListViewModel( TweetModel *model )
{
  ui.statusListView->setModel( model );
}

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
  ui.countdownLabel->setText( ui.statusEdit->isStatusClean() ? QString::number( StatusEdit::STATUS_MAX_LENGTH ) : QString::number( StatusEdit::STATUS_MAX_LENGTH - ui.statusEdit->text().length() ) );
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
  emit resizeView( event->size().width(), event->oldSize().width() );
}

void MainWindow::popupError( const QString &message )
{
  QMessageBox::critical( this, tr("Error"), message );
}

void MainWindow::changeListBackgroundColor(const QColor &newColor )
{
  QPalette palette( ui.statusListView->palette() );
  palette.setColor( QPalette::Base, newColor );
  ui.statusListView->setPalette( palette );
  ui.statusListView->update();
}

void MainWindow::about()
{
  QDialog *dlg = new QDialog( this );
  Ui::AboutDialog aboutUi;
  aboutUi.setupUi( dlg );
  dlg->exec();
  dlg->deleteLater();
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
