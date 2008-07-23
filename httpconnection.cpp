#include "httpconnection.h"

//#define PROXY

HttpConnection::HttpConnection() : QWidget()
{
#ifdef PROXY
  proxy.setType(QNetworkProxy::HttpProxy);
  proxy.setHostName("10.220.1.16");
  proxy.setPort(8080);
  QNetworkProxy::setApplicationProxy(proxy);
#endif
  http = new QHttp(this);
  bytearray = 0;
  buffer = 0;
  
  connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
  connect( http, SIGNAL(dataReadProgress(int, int)), this, SLOT(updateDataReadProgress(int, int)));
  connect( http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
  connect( http, SIGNAL(authenticationRequired(const QString &, quint16, QAuthenticator *)), this, SLOT(slotAuthenticationRequired(const QString &, quint16, QAuthenticator *)));
  connect( &parser, SIGNAL(dataParsed(const QString&)), this, SLOT(forwardDataParsed(const QString&)));
  connect( &parser, SIGNAL(newEntry(const Entry&)), this, SLOT(forwardNewEntry(const Entry&)));
}

void HttpConnection::get( const QString &path )
{
  //url.setUrl( "http://s3.amazonaws.com/twitter_production/profile_images/53492115/avatar_normal.jpg" );
  url.setUrl( path );
  http->setHost( url.host(), QHttp::ConnectionModeHttp);
    
  bytearray = new QByteArray();
  buffer = new QBuffer( bytearray );

  if ( !buffer->open(QIODevice::ReadWrite) )
  {                                     
    QMessageBox::information(this, tr("HTTP"),
      tr("Unable to save the file %1.")
      .arg(buffer->errorString()));
    delete buffer;
    buffer = 0;
    delete bytearray;
    bytearray = 0;
    return;
  }  
  
  //QHttp::ConnectionMode mode = QHttp::ConnectionModeHttp;
  if (!url.userName().isEmpty())
    http->setUser(url.userName(), url.password());
  else
    http->setUser( "", "" );
    
  httpRequestAborted = false;
  QByteArray encodedPath = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
  if ( encodedPath.isEmpty() )
    encodedPath = "/";
  qDebug() << "About to download: " + encodedPath + " from: " + url.host();
  httpGetId = http->get( encodedPath, buffer );
  //httpGetId = http->get( encodedPath, file );
}

void HttpConnection::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  qDebug() << url.path();
  qDebug() << responseHeader.statusCode() << ": " << responseHeader.reasonPhrase() << "\n";
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
    if (buffer) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if(bytearray) {
      delete bytearray;
      bytearray = 0;
    }
  }
}

void HttpConnection::httpRequestFinished(int requestId, bool error)
{
  if (requestId != httpGetId)
    return;
  if (httpRequestAborted) {
    if (buffer) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if(bytearray) {
      delete bytearray;
      bytearray = 0;
    }
    return;
  }
  if (requestId != httpGetId)
    return;
  
  buffer->close();             //  <<<<<<< WTF?! o_O >>>>>>>

  if (error) {
    QMessageBox::information(this, tr("HTTP"),
                             tr("ZOMFG! Download failed: %1.")
                             .arg(http->errorString()));
  }
  QRegExp *resourceType = new QRegExp( "\\.(\\w\\w\\w\\w?)$", Qt::CaseInsensitive );
  int pos = resourceType->indexIn( url.toString() );
  if ( pos > -1 ) {
    if ( resourceType->cap(1) == "xml" ) {
      QXmlInputSource source( buffer );
      QXmlSimpleReader xmlReader;
      xmlReader.setContentHandler( &parser );
      xmlReader.parse( source );
    } else {
      userImage = new QImage();
      userImage->loadFromData( *bytearray, "jpg" );
      emit imageDownloaded( *userImage );
      delete userImage;
      userImage = 0;
    }
  }
  delete buffer;
  buffer = 0;
  delete bytearray;
  bytearray = 0;    
}

void HttpConnection::updateDataReadProgress(int /* bytesRead */, int /* totalBytes */)
{
  if (httpRequestAborted)
    return;
}

void HttpConnection::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  dlg.adjustSize();
  //ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm()).arg(hostName));
  if (dlg.exec() == QDialog::Accepted) {
    authenticator->setUser( ui.loginEdit->text() );
    authenticator->setPassword( ui.passwordEdit->text() );
  }
}

void HttpConnection::forwardDataParsed(const QString &data)
{
  emit dataParsed( data );
}

void HttpConnection::forwardNewEntry( const Entry &entry )
{
  emit newEntry( entry );
}
