#include "mainwindow.h"
#include "statusfilter.h"
#include "xmlparser.h"
#include "entry.h"

#include <QtDebug>
#include <QMessageBox>
#include <QHeaderView>

//#define PROXY

MainWindow::MainWindow() : QWidget(), model( 0,0 )
{
  ui.setupUi( this );
#ifdef PROXY
  proxy.setType(QNetworkProxy::HttpProxy);
  proxy.setHostName("10.220.1.16");
  proxy.setPort(8080);
  QNetworkProxy::setApplicationProxy(proxy);
#endif
  StatusFilter *filter = new StatusFilter();
  http = new QHttp(this);
  
  ui.textEdit->setVisible( false );
  
  ui.statusEdit->installEventFilter( filter );
  ui.statusListView->setModel( &model );
    
  connect( ui.statusEdit, SIGNAL( textChanged(QString) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed(QKeyEvent*) ), this, SLOT( sendStatus(QKeyEvent*) ) );
  connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
  connect( http, SIGNAL(dataReadProgress(int, int)), this, SLOT(updateDataReadProgress(int, int)));
  connect( http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
  connect( http, SIGNAL(authenticationRequired(const QString &, quint16, QAuthenticator *)), this, SLOT(slotAuthenticationRequired(const QString &, quint16, QAuthenticator *)));
  connect( &parser, SIGNAL(dataParsed(const QString&)), this, SLOT(updateText(const QString&)));
  connect( &parser, SIGNAL(newEntry(const Entry&)), this, SLOT(addEntry(const Entry&)));
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
  
  QUrl url( "http://twitter.com/statuses/friends_timeline.xml" );
  QFileInfo fileInfo( "./friends_timeline.xml" );
  QString fileName = fileInfo.fileName();
  
  http->setHost( "www.twitter.com" );
  
  bytearray = new QByteArray();
  buffer = new QBuffer( bytearray );
  textstream = new QTextStream( bytearray );
  /*if (!file->open(QIODevice::WriteOnly))
  {
    QMessageBox::information(this, tr("HTTP"),
      tr("Unable to save the file %1.")
      .arg(file->errorString()));
    delete file;
    file = 0;
    return;
  }*/
  QHttp::ConnectionMode mode = QHttp::ConnectionModeHttp;
  if (!url.userName().isEmpty())
    http->setUser(url.userName(), url.password());

  httpRequestAborted = false;
  QByteArray path = QUrl::toPercentEncoding(url.toString(), "!$&'()*+,;=:@/");
  if (path.isEmpty())
    path = "/";
  qDebug() << url.toString();
  qDebug() << path;
  httpGetId = http->get( path, buffer );
  ui.statusEdit->setText( QString::number( key->key( ) ) + " pressed " + QString::number( httpGetId ) );
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


void MainWindow::httpRequestFinished(int requestId, bool error)
{
  if (requestId != httpGetId)
    return;
  if (httpRequestAborted) {
    if (buffer) {
      buffer->close();
      //file->remove();
      delete buffer;
      buffer = 0;
    }
    return;
  }
  if (requestId != httpGetId)
    return;
  buffer->close();
  if (error) {
    //file->remove();
    QMessageBox::information(this, tr("HTTP"),
                             tr("ZOMFG! Download failed: %1.")
                             .arg(http->errorString()));
  } else {
    ui.statusEdit->setText(tr("Downloaded friends_timeline.xml to current directory."));
  }

  QXmlInputSource source( buffer );
  QXmlSimpleReader reader;
  reader.setContentHandler( &parser );
  reader.parse( source );
  buffer->close();

  delete buffer;
  buffer = 0;
}

void MainWindow::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  switch (responseHeader.statusCode()) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  default:
    QMessageBox::information(this, tr("HTTP"),
                           tr("Download failed: %1.")
                           .arg(responseHeader.reasonPhrase()));
    httpRequestAborted = true;
    http->abort();
  }
}

void MainWindow::updateDataReadProgress(int /* bytesRead */, int /* totalBytes */)
{
  if (httpRequestAborted)
    return;

  ui.countdownLabel->setText( "..." );
}

void MainWindow::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
    //QDialog dlg;
    //Ui::Dialog ui;
    //ui.setupUi(&dlg);
    //dlg.adjustSize();
    //ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm()).arg(hostName));
    //if (dlg.exec() == QDialog::Accepted) {
        authenticator->setUser("d@ayoy.net");
        authenticator->setPassword("vodafone");
    //}
}

void MainWindow::updateText( const QString &text )
{
  ui.textEdit->append( text );
}

void MainWindow::addEntry( const Entry &entry )
{
  QString info("user: %1\nstatus: %2\nimage: %3");
  ui.textEdit->append("BEGIN ENTRY:");
  ui.textEdit->append( info.arg(entry.name()).arg(entry.text()).arg(entry.image()) );
  ui.textEdit->append("END ENTRY\n");
  
  QString iconPath;
  ( entry.name() == "ayoy" ) ? iconPath = "./ayoy.jpg" : iconPath = "./bragi.jpg";
  QIcon *icon = new QIcon( iconPath );
  QStandardItem *szokeItem = new QStandardItem( entry.name() + "\n" + entry.text() );
  szokeItem->setIcon( *icon );
  QSize itemSize( ui.statusListView->size().width() - SCROLLBAR_MARGIN, ICON_SIZE + 10 );

  szokeItem->setSizeHint( itemSize );
  
  model.appendRow( szokeItem ); 
  
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
    lines = (itemSize.height() - 10 ) / fontHeight - 1; // (wysokosc - 10) / wysokość jednej linii - jedna linia na nazwę usera
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
