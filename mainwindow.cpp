#include "mainwindow.h"
#include "statusfilter.h"

#include <QtDebug>


MainWindow::MainWindow() : QWidget(), model( 0,0 )
{
  ui.setupUi( this );
  StatusFilter *filter = new StatusFilter();
  
  //ui.textEdit->setVisible( false );
  
  ui.statusEdit->installEventFilter( filter );
  ui.statusListView->setModel( &model );
    
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed( QKeyEvent* ) ), this, SLOT( sendStatus( QKeyEvent* ) ) );
  connect( &http, SIGNAL( dataParsed( const QString& ) ), this, SLOT( updateText( const QString& ) ) );
  connect( &http, SIGNAL( newEntry( const Entry& ) ), this, SLOT( addEntry( const Entry& ) ));
  connect( &http, SIGNAL( imageDownloaded( const QImage& ) ), this, SLOT( saveImage( const QImage& ) ));
  
}

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
    http.get( "http://twitter.com/statuses/friends_timeline.xml" );
  }
  //ui.statusEdit->setText( QString::number( key->key( ) ) + " pressed " + QString::number( httpGetId ) );
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

void MainWindow::updateText( const QString &text )
{
  ui.textEdit->append( text );
}

void MainWindow::addEntry( const Entry &entry )
{
  userEntry = entry;
  QString info("user: %1\nstatus: %2\nimage: %3");
  //ui.textEdit->append("BEGIN ENTRY:");
  //ui.textEdit->append( info.arg(entry.name()).arg(entry.text()).arg(entry.image()) );
  //ui.textEdit->append( userEntry.image() );
  //ui.textEdit->append("END ENTRY\n");
  
  http.get( userEntry.image() );
  
  //QString iconPath;
  
  //( entry.name() == "ayoy" ) ? iconPath = "./ayoy.jpg" : iconPath = "./bragi.jpg";
  //QIcon *icon = new QIcon( iconPath );
/*  QIcon *icon = new QIcon( QPixmap::fromImage( userImage ) );
  QStandardItem *szokeItem = new QStandardItem( entry.name() + "\n" + entry.text() );
  szokeItem->setIcon( *icon );
  QSize itemSize( ui.statusListView->size().width() - SCROLLBAR_MARGIN, ICON_SIZE + ITEM_SPACING );

  szokeItem->setSizeHint( itemSize );
  
  model.appendRow( szokeItem );*/ 
  
}

void MainWindow::resizeEvent( QResizeEvent *event )
{
  if ( model.rowCount() == 0 )
    return;
    
  int realWidth = event->size().width() - SCROLLBAR_MARGIN - ICON_SIZE;
  
  QRegExp enter( "\n" );
  QString rest;
  QFontMetrics fm( ui.statusListView->font() );
  int fontHeight = fm.height();
  int lines;
  QSize itemSize;
  
  for ( int i = 0; i < model.rowCount(); i++ ) {
    itemSize = model.item(i)->sizeHint();
    lines = (itemSize.height() - ITEM_SPACING ) / fontHeight - 1; // (wysokosc - 10) / wysokość jednej linii - jedna linia na nazwę usera
    itemSize.rwidth() = realWidth + ICON_SIZE;

    rest = model.item(i)->text().right( model.item(i)->text().length() - enter.indexIn( model.item(i)->text() ) - 1 );

    if ( fm.width( rest ) > lines * realWidth ) {
      itemSize.rheight() += fontHeight;
    } else if ( ( lines > 2 ) && ( fm.width( rest ) < (lines-1) * realWidth ) ) {
      itemSize.rheight() -= fontHeight;
    }
    model.item(i)->setSizeHint( itemSize );
  }
}

void MainWindow::saveImage ( const QImage &image ) {
  //userImage = image;
  //userImage = new QImage( image );
  QIcon *icon = new QIcon( QPixmap::fromImage( image ) );
  QStandardItem *newItem = new QStandardItem( userEntry.name() + "\n" + userEntry.text() );
  newItem->setIcon( *icon );
  QSize itemSize( ui.statusListView->size().width() - SCROLLBAR_MARGIN, ICON_SIZE + ITEM_SPACING );

  newItem->setSizeHint( itemSize );
  
  model.appendRow( newItem ); 
}
