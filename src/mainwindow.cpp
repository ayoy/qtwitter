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
#include <QDesktopWidget>

const QString MainWindow::APP_VERSION = "0.4.2_pre1";

MainWindow::MainWindow( QWidget *parent ) :
    QWidget( parent ),
    resetUiWhenFinished( false )
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
  connect( ui.statusEdit, SIGNAL(errorMessage(QString)), this, SLOT(popupError(QString)) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( filter, SIGNAL( escPressed() ), ui.statusEdit, SLOT( cancelEditing() ) );
  connect( this, SIGNAL(addReplyString(QString)), ui.statusEdit, SLOT(addReplyString(QString)) );
  connect( this, SIGNAL(addRetweetString(QString)), ui.statusEdit, SLOT(addRetweetString(QString)) );

  QShortcut *typeShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ), this );
  connect( typeShortcut, SIGNAL(activated()), ui.statusEdit, SLOT(setFocus()) );
  QShortcut *hideShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_H ), this );
  connect( hideShortcut, SIGNAL(activated()), this, SLOT(hide()) );
  QShortcut *quitShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ), this );
  connect( quitShortcut, SIGNAL(activated()), qApp, SLOT(quit()) );
#ifdef Q_WS_MAC
  ui.settingsButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Comma ) );
#else
  ui.settingsButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
#endif
  ui.updateButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ) );
  ui.homeButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );

  trayIcon = new QSystemTrayIcon( this );
  trayIcon->setIcon( QIcon( ":/icons/twitter_48.png" ) );

  QObject::connect( trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)) );
#ifndef Q_WS_MAC
  QMenu *trayMenu = new QMenu( this );
  trayMenu = new QMenu( this );
  QAction *quitaction = new QAction( tr( "Quit" ), trayMenu);
  QAction *settingsaction = new QAction( tr( "Settings" ), trayMenu);
  settingsaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
  quitaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ) );

  QObject::connect( quitaction, SIGNAL(triggered()), qApp, SLOT(quit()) );
  QObject::connect( settingsaction, SIGNAL(triggered()), this, SIGNAL(settingsDialogRequested()) );
  QObject::connect( settingsaction, SIGNAL(triggered()), this, SLOT(show()) );

  trayMenu->addAction(settingsaction);
  trayMenu->addAction(quitaction);
  trayIcon->setContextMenu( trayMenu );

  trayIcon->setToolTip( "qTwitter" );
#endif
  trayIcon->show();
  emit updateTweets();
}

MainWindow::~MainWindow() {}

StatusList* MainWindow::getListView()
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
    case QSystemTrayIcon::Trigger:
#ifdef Q_WS_WIN
    if ( !isVisible() ) {
#else
    if ( !isVisible() || !QApplication::activeWindow() ) {
#endif
      show();
        raise();
        activateWindow();
      } else {
        hide();
      }
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
  resetUiWhenFinished = true;
  emit post( ui.statusEdit->text().toUtf8() );
}

void MainWindow::resetStatusEdit()
{
  if ( resetUiWhenFinished ) {
    resetUiWhenFinished = false;
    ui.statusEdit->cancelEditing();
  }
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
  QMessageBox::information( this, tr("Error"), message );
}

void MainWindow::popupMessage( int statusesCount, QStringList namesForStatuses, int messagesCount, QStringList namesForMessages )
{
  QRegExp rx( ", " );
  QString message;
#ifdef Q_WS_MAC
  QString title;
  if ( !namesForStatuses.isEmpty() ) {
    title.append( QString::number( statusesCount ) + " " );
    statusesCount == 1 ? title.append( tr( "new tweet" ) ) : title.append( tr( "new tweets" ) );
    message.append( tr( "from" ) + " " + namesForStatuses.join( ", " ) + "." );
    message.replace( rx.lastIndexIn( message ), rx.pattern().length(), " " + tr( "and" ) + " " );
    trayIcon->showMessage( title, message, QSystemTrayIcon::Information );
  }
  if ( !namesForMessages.isEmpty() ) {
    message.clear();
    title.clear();
    title.append( QString::number( messagesCount ) + " " );
    messagesCount == 1 ? title.append( tr( "new message" ) ) : title.append( tr( "new messages" ) );
    message.append( tr( "from" ) + " " + namesForMessages.join(", ") + "." );
    message.replace( rx.lastIndexIn( message ), rx.pattern().length(), " " + tr( "and" ) + " " );
    trayIcon->showMessage( title, message, QSystemTrayIcon::Information );
  }
#else
  if ( !namesForStatuses.isEmpty() ) {
    message.append( QString::number( statusesCount ) + " " );
    statusesCount == 1 ? message.append( tr( "new tweet from" ) ) : message.append( tr( "new tweets from" ) );
    message.append( " " + namesForStatuses.join( ", " ) + "." );
    message.replace( rx.lastIndexIn( message ), rx.pattern().length(), " " + tr( "and" ) + " " );
  }
  if ( !namesForMessages.isEmpty() ) {
    if ( !namesForStatuses.isEmpty() ) {
      message.append( "\n\n" );
    }
    message.append( QString::number( messagesCount ) + " " );
    messagesCount == 1 ? message.append( tr( "New message from" ) ) : message.append( tr( "New messages from" ) );
    message.append( " " + namesForMessages.join( ", " ) + "." );
    message.replace( rx.lastIndexIn( message ), rx.pattern().length(), " " + tr( "and" ) + " " );
  }
  if ( !namesForMessages.isEmpty() || !namesForStatuses.isEmpty() ) {
    trayIcon->showMessage( tr( "News from qTwitter" ), message, QSystemTrayIcon::Information );
  }
#endif
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
  aboutUi.textBrowser->setHtml( aboutUi.textBrowser->toHtml().arg( APP_VERSION ) );
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
