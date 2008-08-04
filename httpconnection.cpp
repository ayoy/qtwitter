#include "httpconnection.h"

HttpConnection::HttpConnection() : QThread()
{
  http = new QHttp(this);
  bytearray = 0;
  buffer = 0;
  
  connect( http, SIGNAL(requestStarted(int)), this, SLOT(httpRequestStarted(int)));  
  connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
  connect( http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
  connect( http, SIGNAL(authenticationRequired(const QString &, quint16, QAuthenticator *)), this, SLOT(slotAuthenticationRequired(const QString &, quint16, QAuthenticator *)));
}

HttpConnection::~HttpConnection() {
  if ( http ) {
    delete http;
    http = 0;
  }
  if( buffer ) {
    delete buffer;
    buffer = 0;
  }
  if( bytearray ) {
    delete bytearray;
    bytearray = 0;
  }
}

void HttpConnection::httpRequestStarted( int /*requestId*/ ) {
  qDebug() << "The request has started";
}

void HttpConnection::run() {
  //wc.wakeAll();
  //qDebug() << "Wait condition released";
}

void HttpConnection::setUrl( const QString &path ) {
  url.setUrl( path );
}

QByteArray HttpConnection::prepareRequest( const QString &path ) {
  url.setUrl( path );
  //url.setUrl( "http://s3.amazonaws.com/twitter_production/profile_images/53492115/avatar2_normal.jpg" );
  http->setHost( url.host(), QHttp::ConnectionModeHttp);
    
  bytearray = new QByteArray();
  buffer = new QBuffer( bytearray );

  if ( !buffer->open(QIODevice::ReadWrite) )
  {                                     
    emit errorMessage( tr("Unable to open device: ") + buffer->errorString() );
    delete buffer;
    buffer = 0;
    delete bytearray;
    bytearray = 0;
    return "invalid";
  }  
  
  if (!url.userName().isEmpty())
    http->setUser(url.userName(), url.password());
  else
    http->setUser( "", "" );
    
  httpRequestAborted = false;
  QByteArray encodedPath = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
  if ( encodedPath.isEmpty() )
    encodedPath = "/";
  qDebug() << "About to download: " + encodedPath + " from: " + url.host();
  return encodedPath;
}

void HttpConnection::get( const QString &path )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  httpGetId = http->get( encodedPath, buffer );
  qDebug() << httpGetId;
}

void HttpConnection::post( const QString &path, const QByteArray &status )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  httpGetId = http->post( encodedPath, status, buffer );
  qDebug() << httpGetId;
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
