#include "httpconnection.h"

//#define PROXY

HttpConnection::HttpConnection() : QThread()
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
}

void HttpConnection::run() {
  get( url.toString() );
}

void HttpConnection::setUrl( const QString &path ) {
  url.setUrl( path );
}

void HttpConnection::get( const QString &path )
{
  url.setUrl( path );
  //url.setUrl( "http://s3.amazonaws.com/twitter_production/profile_images/53492115/avatar2_normal.jpg" );
  http->setHost( url.host(), QHttp::ConnectionModeHttp);
    
  bytearray = new QByteArray();
  buffer = new QBuffer( bytearray );

  if ( !buffer->open(QIODevice::ReadWrite) )
  {                                     
    emit errorMessage( "Unable to save the file " + buffer->errorString() );
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
  case 404:                   // Not Found
    break;
  default:
    emit errorMessage( "Download failed: " + responseHeader.reasonPhrase() );
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

void HttpConnection::requestFinished(int requestId, bool error)
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
  
  buffer->close(); 
  
  if (error) {
    emit errorMessage( "Download failed: " + http->errorString() );
  }
   
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
