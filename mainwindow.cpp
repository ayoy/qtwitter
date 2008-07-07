#include "mainwindow.h"
#include "statusfilter.h"
#include "xmlparser.h"

#include <QtDebug>
#include <QMessageBox>

//#define PROXY

MainWindow::MainWindow() : QWidget()
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
  ui.statusEdit->installEventFilter( filter );
  connect( ui.statusEdit, SIGNAL( textChanged(QString) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed(QKeyEvent*) ), this, SLOT( sendStatus(QKeyEvent*) ) );
  connect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(httpRequestFinished(int, bool)));
  connect(http, SIGNAL(dataReadProgress(int, int)),this, SLOT(updateDataReadProgress(int, int)));
  connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
  connect(http, SIGNAL(authenticationRequired(const QString &, quint16, QAuthenticator *)),this, SLOT(slotAuthenticationRequired(const QString &, quint16, QAuthenticator *)));
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
  
  file = new QFile( "friends_timeline.xml" );
  if (!file->open(QIODevice::WriteOnly))
  {
    QMessageBox::information(this, tr("HTTP"),
      tr("Unable to save the file %1.")
      .arg(file->errorString()));
    delete file;
    file = 0;
    return;
  }
  QHttp::ConnectionMode mode = QHttp::ConnectionModeHttp;
  if (!url.userName().isEmpty())
    http->setUser(url.userName(), url.password());

  httpRequestAborted = false;
  QByteArray path = QUrl::toPercentEncoding(url.toString(), "!$&'()*+,;=:@/");
  if (path.isEmpty())
    path = "/";
  qDebug() << url.toString();
  qDebug() << path;
  httpGetId = http->get( path, file );
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
    if (file) {
      file->close();
      file->remove();
      delete file;
      file = 0;
    }
    return;
  }
  if (requestId != httpGetId)
    return;
  file->close();
  if (error) {
    file->remove();
    QMessageBox::information(this, tr("HTTP"),
                             tr("ZOMFG! Download failed: %1.")
                             .arg(http->errorString()));
  } else {
    ui.statusEdit->setText(tr("Downloaded friends_timeline.xml to current directory."));
  }

  if ( !file->open( QIODevice::ReadOnly ) )
  {
    QMessageBox::information(this, tr("HTTP"),
      tr("Unable to save the file %1.")
      .arg(file->errorString()));
    delete file;
    file = 0;
    return;
  }

  QXmlInputSource source( file );
  XmlParser parser;
  QXmlSimpleReader reader;
  reader.setContentHandler( &parser );
  reader.parse( source );
  file->close();

  delete file;
  file = 0;
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

void MainWindow::updateDataReadProgress(int bytesRead, int totalBytes)
{
  if (httpRequestAborted)
    return;

  ui.countdownLabel->setText( "..." );
}

void MainWindow::slotAuthenticationRequired(const QString &hostName, quint16, QAuthenticator *authenticator)
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
  