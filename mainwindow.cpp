#include "mainwindow.h"
#include "statusfilter.h"

#include <QtDebug>

MainWindow::MainWindow() : QWidget()
{
  ui.setupUi( this );
  //isStatusClean = true;
  StatusFilter *filter = new StatusFilter();
  ui.statusEdit->installEventFilter( filter );
  connect( ui.statusEdit, SIGNAL( textChanged(QString) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed(QKeyEvent*) ), this, SLOT( sendStatus(QKeyEvent*) ) );
}

/*void MainWindow::addItem()
{
  EditDialog dlg( this );

  if( dlg.exec() == QDialog::Accepted )
    ui.list->addItem( dlg.name() + " -- " + dlg.number() );
}*/

void MainWindow::changeLabel()
{
  ui.countdownLabel->setText( QString::number( STATUS_MAX_LEN - ui.statusEdit->text().length() ) );
  ui.statusEdit->setStatusClean( false );
}

void MainWindow::sendStatus( QKeyEvent *key )
{
  if ( key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return )
  {
    ui.statusEdit->setText(QString::number(key->key( )) + " pressed");
  }
}

void MainWindow::resetStatus()
{
  if ( ui.statusEdit->text().length() == 0 )
  {
    ui.statusEdit->setText( "What are you doing?" );
    ui.countdownLabel->setText( QString::number(STATUS_MAX_LEN) );
    ui.statusEdit->setStatusClean( true );
  }
}
//  http = new QHttp( "www.twitter.com" );
//  QHttpRequestHeader header( "GET", "/statuses/friends_timeline.xml" );
//  header.setValue( "Host", "www.twitter.com" );
//  http->request( header );
