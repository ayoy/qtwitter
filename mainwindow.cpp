#include "mainwindow.h"


MainWindow::MainWindow() : QWidget(), model( 0, 0, this )
{
  ui.setupUi( this );
  filter = new StatusFilter();
  fm = new QFontMetrics( ui.statusListView->font() );
  ui.statusEdit->installEventFilter( filter );
  ui.statusListView->setModel( &model );
  proxy.setType( QNetworkProxy::NoProxy );
  
  connect( ui.updateButton, SIGNAL( clicked() ), this, SLOT( updateTweets() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), this, SLOT( openSettings() ) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( &threadingEngine, SIGNAL( errorMessage( const QString& ) ), this, SLOT( popupError( const QString& ) ) );
  connect( &threadingEngine, SIGNAL( readyToDisplay( const QList<Entry>&, const QMap<QString, QImage>& ) ), this, SLOT( display( const QList<Entry>&, const QMap<QString, QImage>& ) ) );

  updateTweets();
}

MainWindow::~MainWindow() {
  if ( filter ) {
    delete filter;
    filter = 0;
  }
  if ( fm ) {
    delete fm;
    fm = 0;
  }
}

void MainWindow::changeLabel() {
  ui.countdownLabel->setText( QString::number( STATUS_MAX_LEN - ui.statusEdit->text().length() ) );
}

void MainWindow::updateTweets() {
  ui.updateButton->setEnabled( false );
  threadingEngine.get( "http://twitter.com/statuses/friends_timeline.xml" );
}

void MainWindow::sendStatus() {
  QByteArray status( "status=" );
  status.append( ui.statusEdit->text().toUtf8() );
  status.append( "&source=qtwitter" );
  qDebug() << status;
  const QString path("http://twitter.com/statuses/update.xml");
  threadingEngine.post( path, status );
}

void MainWindow::resetStatus() {
  if ( ui.statusEdit->text().length() == 0 )
  {
    ui.statusEdit->initialize();
    ui.countdownLabel->setText( QString::number(STATUS_MAX_LEN) );
  }
}

void MainWindow::checkAlign( int width ) {
  if ( model.rowCount() == 0 )
    return;
    
  int realWidth =  width - SCROLLBAR_MARGIN - ICON_SIZE;
  
  QRegExp enter( "\n" );
  QString rest;
  int fontHeight = fm->height();
  int lines;
  QSize itemSize;
  
  for ( int i = 0; i < model.rowCount(); i++ ) {
    itemSize = model.item(i)->sizeHint();
    lines = (itemSize.height() - ITEM_SPACING ) / fontHeight - 1; // (field height - 10) / line height - 1 line for username
    itemSize.rwidth() = realWidth + ICON_SIZE;

    rest = model.item(i)->text().right( model.item(i)->text().length() - enter.indexIn( model.item(i)->text() ) - 1 );

    if ( fm->width( rest ) > lines * realWidth ) {
      itemSize.rheight() += fontHeight;
    } else if ( ( lines > 2 ) && ( fm->width( rest ) < (lines-1) * realWidth ) ) {
      itemSize.rheight() -= fontHeight;
    }
    model.item(i)->setSizeHint( itemSize );
  }
}

void MainWindow::resizeEvent( QResizeEvent *event ) {
  checkAlign( event->size().width() );
}

void MainWindow::display( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash ) {
  model.clear();
  for ( int i = 0; i < entries.size(); i++ ) {
    QIcon *icon = new QIcon( QPixmap::fromImage( imagesHash[ entries[i].image() ] ) );
    QStandardItem *newItem = new QStandardItem( entries[i].name() + "\n" + entries[i].text() );
    newItem->setIcon( *icon );
    QSize itemSize( ui.statusListView->size().width() - SCROLLBAR_MARGIN, ICON_SIZE + ITEM_SPACING );
    
    newItem->setSizeHint( itemSize );
  
    model.appendRow( newItem );
  }
  checkAlign( size().width() );
  unlockState();
}

void MainWindow::unlockState() {
  ui.updateButton->setEnabled( true );
  if ( !ui.statusEdit->isEnabled() ) {
    ui.statusEdit->setEnabled( true );
    ui.statusEdit->initialize();
  }
}

void MainWindow::popupError( const QString &message ) {
  QMessageBox::information( this, tr("Error"), message );
  unlockState();
}

void MainWindow::openSettings() {
  QDialog dlg;
  ui_s.setupUi(&dlg);
  if ( proxy.type() != QNetworkProxy::NoProxy ) {
    ui_s.proxyBox->setChecked( true );
    ui_s.hostEdit->setEnabled( true );
    ui_s.portEdit->setEnabled( true );
  }
  ui_s.hostEdit->setText( proxy.hostName() );
  if ( proxy.port() ) {
    ui_s.portEdit->setText( QString::number( proxy.port() ) );
  }
  if ( dlg.exec() == QDialog::Accepted ) {
    if ( ui_s.proxyBox->isChecked() ) {
      proxy.setType( QNetworkProxy::HttpProxy );
      proxy.setHostName( ui_s.hostEdit->text() );
      proxy.setPort( ui_s.portEdit->text().toInt() );
    } else {
      proxy.setType( QNetworkProxy::NoProxy );
    }
    QNetworkProxy::setApplicationProxy( proxy );
  }
}

